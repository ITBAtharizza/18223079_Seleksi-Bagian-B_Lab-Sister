#include <stdio.h>

#define MAX_DIGITS 1001

long long tambah(long long a, long long b) {
    long long carry;
tambah_loop:
    long long sum = a ^ b;
    carry = (a & b) << 1;
    a = sum;
    b = carry;
    if (b) goto tambah_loop;
    return a;
}

long long negasi(long long n) {
    return tambah(~n, 1);
}

long long kali(long long a, long long b) {
    long long hasil = 0;
kali_loop:
    if (!b) goto kali_end;
    if (b & 1) {
        hasil = tambah(hasil, a);
    }
    a = a << 1;
    b = b >> 1;
    goto kali_loop;
kali_end:
    return hasil;
}

void string_ke_array(char* str, char* arr, int* len) {
    long long i = 0, j;
str_len_loop:
    if (!str[i]) goto str_len_end;
    i = tambah(i, 1);
    goto str_len_loop;
str_len_end:
    *len = i;
    j = tambah(*len, negasi(1));
    i = 0;
convert_loop:
    if ((j >> 63) & 1) goto convert_end;
    arr[i] = str[j] & 0x0F;
    i = tambah(i, 1);
    j = tambah(j, negasi(1));
    goto convert_loop;
convert_end:
    return;
}

void cetak_array(long long* arr, int len) {
    long long i = tambah(len, negasi(1));
find_start_loop:
    if ( i && !(arr[i]) ) { 
        i = tambah(i, negasi(1));
        goto find_start_loop;
    }
print_loop:
    if ((i >> 63) & 1) goto print_end;
    printf("%lld", arr[i]);
    i = tambah(i, negasi(1));
    goto print_loop;
print_end:
    printf("\n");
}

void bagi_mod_10(long long val, long long* quot, long long* rem) {
    *quot = 0;
    *rem = val;
    long long sepuluh_negatif = negasi(10);
div_loop:
    long long sisa_cek = tambah(*rem, sepuluh_negatif);
    if ((sisa_cek >> 63) & 1) { 
        goto div_end;
    }
    *rem = sisa_cek;
    *quot = tambah(*quot, 1);
    goto div_loop;
div_end:
    return;
}

void kali_bignum(char* a, int len_a, char* b, int len_b, long long* hasil, int* len_hasil) {
    long long i = 0, j = 0;
    long long max_len_hasil = tambah(len_a, len_b);
    long long k = 0;
init_loop:
    if ( !((tambah(k, negasi(max_len_hasil)) >> 63) & 1) ) goto init_end;
    hasil[k] = 0;
    k = tambah(k, 1);
    goto init_loop;
init_end:

outer_loop:
    if ( !((tambah(i, negasi(len_a)) >> 63) & 1) ) goto outer_loop_end;
    j = 0;
inner_loop:
    if ( !((tambah(j, negasi(len_b)) >> 63) & 1) ) goto inner_loop_end;
    
    long long produk = kali(a[i], b[j]);
    long long pos = tambah(i, j);
    hasil[pos] = tambah(hasil[pos], produk);
    
    j = tambah(j, 1);
    goto inner_loop;
inner_loop_end:
    i = tambah(i, 1);
    goto outer_loop;
outer_loop_end:

    long long carry = 0;
    k = 0;
normalize_loop:
    if ( !((tambah(k, negasi(max_len_hasil)) >> 63) & 1) ) goto normalize_end;
    
    long long jumlah_total = tambah(hasil[k], carry);
    long long quot_baru, rem_baru;
    
    bagi_mod_10(jumlah_total, &quot_baru, &rem_baru);
    
    hasil[k] = rem_baru;
    carry = quot_baru;
    
    k = tambah(k, 1);
    goto normalize_loop;
normalize_end:

    *len_hasil = max_len_hasil;
}

int main() {
    char str1[MAX_DIGITS], str2[MAX_DIGITS];
    char num1[MAX_DIGITS], num2[MAX_DIGITS];
    long long hasil[(MAX_DIGITS << 1)]; 
    int len1, len2, len_hasil;

    scanf("%s %s", str1, str2);
    
    if (!(str1[0] ^ '0') && !(str1[1] ^ '\0')) { 
        printf("0\n"); 
        return 0; 
    }
    if (!(str2[0] ^ '0') && !(str2[1] ^ '\0')) { 
        printf("0\n"); 
        return 0; 
    }

    string_ke_array(str1, num1, &len1);
    string_ke_array(str2, num2, &len2);

    kali_bignum(num1, len1, num2, len2, hasil, &len_hasil);
    
    cetak_array(hasil, len_hasil);

    return 0;
}