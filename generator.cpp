#include<bits/stdc++.h>
#include<x86intrin.h>
#include<format>
using namespace std;

typedef unsigned long long ull;

void printbb(ull p){
    cout << "+---+---+---+---+---+---+---+---+\n";
    for(int r=7;r>-1;r--){
        for(int f=0;f<8;f++){
            cout << "| " << ((p & (1ull << (r * 8 + f))) ? "X" : " ") << " ";
        } cout << "|\n";
        cout << "+---+---+---+---+---+---+---+---+\n";
    }
}

ull rmasks[64];

void rookMasks(){
    for(int r=0;r<8;r++){
        for(int f=0;f<8;f++){
            rmasks[r*8+f] = (0xFFull << (r * 8)) ^ (0x0101010101010101ull << f);
            rmasks[r*8+f] &= ~(0xFF818181818181FF & ~((0x0001010101010100ull << f) | (0x7Eull << (r * 8))));
        }
    }
}

ull rlookups[64*4096];

void rookGen(){
    for(unsigned i=0;i<4096;i++){
        for(int m=0;m<64;m++) {
            int r = m / 8, f = m % 8;
            ull occ = _pdep_u64(i, rmasks[m]);
            //printbb(occ);
            for(int k=r+1;k<8;k++){
                rlookups[m*4096+i] |= 1ull << (k*8+f);
                if(occ & (1ull << (k*8+f))) break;
            }
            for(int k=r-1;k>-1;k--){
                rlookups[m*4096+i] |= 1ull << (k*8+f);
                if(occ & (1ull << (k*8+f))) break;
            }
            for(int k=f+1;k<8;k++){
                rlookups[m*4096+i] |= 1ull << (r*8+k);
                if(occ & (1ull << (r*8+k))) break;
            }
            for(int k=f-1;k>-1;k--){
                rlookups[m*4096+i] |= 1ull << (r*8+k);
                if(occ & (1ull << (r*8+k))) break;
            }
            rlookups[m*4096+i] &= ~(1ull << m);
        }
    }
}

ull bmasks[64];

void bishopMasks(){
    for(int r=0;r<8;r++) for(int f=0;f<8;f++){
        if (f+r-7 > 0) bmasks[r*8+f] ^= (0x0102040810204080ull << ((f+r-7)*8));
        else bmasks[r*8+f] ^= (0x0102040810204080ull >> ((7-f-r)*8));
        if (r-f > 0) bmasks[r*8+f] ^= (0x8040201008040201ull << ((r-f)*8));
        else bmasks[r*8+f] ^= (0x8040201008040201ull >> ((f-r)*8));
        bmasks[r*8+f] &= ~0xFF818181818181FF;
    }
}

ull blookups[64*512];

void bishopGen(){
    for(unsigned i=0;i<512;i++){
        for(int m=0;m<64;m++) {
            int r = m / 8, f = m % 8;
            ull occ = _pdep_u64(i, bmasks[m]);
            //printbb(occ);
            for(int k=r,l=f;k<8 && l<8;k++,l++){
                blookups[m*512+i] |= 1ull << (k*8+l);
                if(occ & (1ull << (k*8+l))) break;
            }
            for(int k=r,l=f;k>-1 && l<8;k--,l++){
                blookups[m*512+i] |= 1ull << (k*8+l);
                if(occ & (1ull << (k*8+l))) break;
            }
            for(int k=r,l=f;k<8 && l>-1;k++,l--){
                blookups[m*512+i] |= 1ull << (k*8+l);
                if(occ & (1ull << (k*8+l))) break;
            }
            for(int k=r,l=f;k>-1 && l>-1;k--,l--){
                blookups[m*512+i] |= 1ull << (k*8+l);
                if(occ & (1ull << (k*8+l))) break;
            }
            blookups[m*512+i] &= ~(1ull << m);
        }
    }
}

int main(){
    rookMasks();
    rookGen();
    bishopMasks();
    bishopGen();
    ofstream o;
    o.open("map_output.cpp");
    o << "#include<bits/stdc++.h>\nusing namespace std;\n\ntypedef unsigned long long ull;\n\nconstexpr ull rMap[64*4096] = {\n    " << format("{:#x}", rlookups[0]);
    for (int i=1;i<64*4096;i++){
        o << format(",{:#x}", rlookups[i]);
    } o << "\n};\nconstexpr ull rMask[64] = {\n    " << format("{:#x}", rmasks[0]);
    for (int i=1;i<64;i++){
        o << format(",{:#x}", rmasks[i]);
    } o << "\n};\nconstexpr ull bMask[64] = {\n    " << format("{:#x}", bmasks[0]);
    for (int i=1;i<64;i++){
        o << format(",{:#x}", bmasks[i]);
    } o << "\n};\nconstexpr ull bMap[64*512] = {\n    " << format("{:#x}", blookups[0]);
    for (int i=1;i<64*512;i++){
        o << format(",{:#x}", blookups[i]);
    } o << "\n};\n";
    // for(int i=0;i<64;i++){
    //     printbb(bmasks[i]);
    // }
}