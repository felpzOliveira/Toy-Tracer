#pragma once
#include <cutil.h>
#include <stdio.h>
#include <geometry.h>

template<typename T, class C> inline __bidevice__
bool QuickSort(T *arr, int elements, C compare){
#define  MAX_LEVELS  1000
    int beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R;
    T piv;
    beg[0] = 0;
    end[0] = elements;
    while(i >= 0){
        L = beg[i]; 
        R = end[i]-1;
        if(L < R){
            piv = arr[L]; 
            if(i == MAX_LEVELS-1) return false;
            
            while(L < R){
                while(compare(&arr[R], &piv) && L < R) R--; if (L < R) arr[L++] = arr[R];
                while(compare(&piv, &arr[L]) && L < R) L++; if (L < R) arr[R--] = arr[L]; 
            }
            
            arr[L] = piv; 
            beg[i+1] = L+1; 
            end[i+1] = end[i]; 
            end[i++] = L; 
        }else{
            i--; 
        }
    }
    
    return true;
}

inline __bidevice__ void PrintBinary(unsigned int n){
    char arr[64], rev[64];
    int i = 0, it = 0, r;
    while(n != 0){
        r = n % 2;
        arr[i++] = r + '0';
        n /= 2;
    }
    
    while(i--){
        rev[it++] = arr[i];
    }
    
    rev[it++] = 0;
    printf("%s\n", rev);
}

inline __bidevice__ unsigned int LeftShift3(unsigned int x){
    Assert(x <= (1 << 10));
    if(x == (1 << 10)) --x;
    x = (x | (x << 16)) & 0x30000ff;
    x = (x | (x << 8)) & 0x300f00f;
    x = (x | (x << 4)) & 0x30c30c3;
    x = (x | (x << 2)) & 0x9249249;
    return x;
}

inline __bidevice__ unsigned int MortonCode3D(Float x, Float y, Float z){
    Assert(x >= 0);
    Assert(y >= 0);
    Assert(z >= 0);
    return (LeftShift3(z) << 2) | (LeftShift3(y) << 1) | LeftShift3(x);
}