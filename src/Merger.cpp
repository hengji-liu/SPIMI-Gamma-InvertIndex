﻿#include "../include/Merger.h"

Merger::Merger()
{
    //ctor
}

Merger::~Merger()
{
    //dtor
}
void Merger::readTerm(ifstream &in, char *term) {
    char termLength =  -1;
    in.read(&termLength, sizeof(char));
    if (termLength == -1) {
        term[0] = 0; // \0 indicates the end of string
        return;
    }
    in.read(term, termLength);
    term[(int)termLength] = 0;
}
void Merger::writeToFile(ofstream &out, char* term, int df, IndexList &il) {
    char len = strlen(term);
    if (len == 0)cout<<"error!!"<<endl;
    // write term length and term
    out.write(&len, sizeof(char));
    out.write(term, len * sizeof(char));
    // write doc freq
    out.write((char *)&df, sizeof(int));
    // write posting list
    il.writeOffset(out);
    il.writeToFile(out);
}
void Merger::writeRest(ofstream &out, ifstream &in, char *s) {
    char len = strlen(s);
    out.write(&len, sizeof(char));
    out.write(s, sizeof(char) * len);
    char c;
    while(!in.eof()) {
        c = -1;
        in.read(&c, sizeof(char));
        if (c == -1)break;
        out.write(&c, sizeof(char));
    }
}
void Merger::mergeTwo(const char *file1, const char *file2, char *outfile) {
    ifstream in1(file1, ios::binary|ios::in), in2(file2, ios::binary|ios::in);
    ofstream out(outfile, ios::binary|ios::out);
    cout<<file1<<" "<<file2<<endl;
    char s1[100],s2[100];
    readTerm(in1, s1);
    readTerm(in2, s2);
    int len, df;
    IndexList il, tmp;
    itemNum = 0;
    while(s1[0] != 0 && s2[0] != 0) {
        //cout<<s1<<" "<<s2<<endl;
        if (strcmp(s1, s2) < 0) {
            in1.read((char *)&df, sizeof(int));
            in1.read((char *)&len, sizeof(int));
            il.readFromFile(in1, len);
            writeToFile(out, s1, df, il);
            readTerm(in1, s1);
            itemNum++;
        } else if (strcmp(s1, s2) > 0) {
            in2.read((char *)&df, sizeof(int));
            in2.read((char *)&len, sizeof(int));
            il.readFromFile(in2, len);
            writeToFile(out, s2, df, il);
            readTerm(in2, s2);
            itemNum++;
        } else {
            in1.read((char *)&df, sizeof(int));
            in1.read((char *)&len, sizeof(int));
            il.readFromFile(in1, len);
            int t, df1;
            in2.read((char *)&df1, sizeof(int));
            in2.read((char *)&t, sizeof(int));
            tmp.readFromFile(in2, t);
            vector<int> v = tmp.decode();
            int size = v.size();
            for (int i = 0; i < size; i++) {
                il.push(v[i]);
            }
            writeToFile(out, s1, df + df1, il);
            readTerm(in1, s1);
            readTerm(in2, s2);
            itemNum++;
        }
    }
    if (s1[0] != 0) {
        writeRest(out, in1, s1);
    }
    if (s2[0] != 0) {
        writeRest(out, in2, s2);
    }
    in1.close();
    in2.close();
    out.close();
}

string Merger::merge(string path) {
    Util util;
    vector<string> v;
    char name[100];
    int idx = 0;
    while(true) {
        if(v.size() <= 1) {
            v = util.getFiles(path);
            for (int i = 0; i < (int)v.size(); i++) {
                v[i] = path + "/" + v[i];
            }
            if (v.size() <= 1) break;
        }
        sprintf(name, "./tmp/m%d", idx++);
        mergeTwo(v[0].c_str(),v[1].c_str(), name);
        util.delFile(v[0]);
        util.delFile(v[1]);
        v.erase(v.begin());
        v.erase(v.begin());
    }
    return v[0];
}
