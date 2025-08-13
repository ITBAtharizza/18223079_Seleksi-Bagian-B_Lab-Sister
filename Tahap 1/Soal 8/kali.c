#include <stdio.h>

// Definisi konstanta dan makro
#define MAX_N 2097152       // 2^21
#define MOD 998244353       // Modulus Prima untuk NTT
#define G 3                 // Primitive Root dari MOD
#define GI 332748118        // Invers Modular dari G 

// Barrett Reduction constants untuk MOD = 998244353
// Dengan modulus 30 bit, k = 30
// r = floor(2 ^ 60 / MOD) = 1161330260266822179
#define BARRETT_K 30
#define BARRETT_R 1161330260266822179ULL
#define BARRETT_2K 60

#define FORWARD_NTT 1
#define INVERSE_NTT ~0 

// Deklarasi array global
char s1[MAX_N], s2[MAX_N];
long long a[MAX_N], b[MAX_N];
int rev[MAX_N];

// BAGIAN 1: FUNGSI ARITMETIKA BITWISE DASAR
// Penjumlahan bitwise
long long func_add(long long a, long long b) {
    long long carry;
    add_bitwise_loop:
    if (b) {
        carry = (a & b) << 1;
        a = a ^ b;
        b = carry;
        goto add_bitwise_loop;
    }
    return a;
}

// Pengurangan bitwise
long long func_sub(long long a, long long b) {
    return func_add(a, func_add(~b, 1));
}

// BAGIAN 2: FUNGSI HELPER
// Memeriksa apakah a < b
long long is_less_than(long long a, long long b) {
    return func_sub(a, b) >> 63;
}

// Perkalian bitwise sederhana
long long simple_multiply(long long a, long long b) {
    long long res = 0;
    mul_loop:
    if (b) {
        if (b & 1) {
            res = func_add(res, a);
        }
        a = a << 1;
        b = b >> 1;
        goto mul_loop;
    }
    return res;
}

// Barrett reduction untuk mengganti operasi modulo
long long barrett_reduce(long long a) {
    // Kasus sudah dalam range
    if (!is_less_than(a, 0)) {
        if (is_less_than(a, MOD)) {
            return a;
        }
    }
    
    // Jika negatif, tambahkan MOD sampai positif
    if (is_less_than(a, 0)) {
        add_mod_loop:
        if (is_less_than(a, 0)) {
            a = func_add(a, MOD);
            goto add_mod_loop;
        }
        return a;
    }
    
    // Barrett reduction untuk a lebih besar sama dengan MOD
    // Hitung q = floor(a x r / 2^60)    
    // Untuk menghindari overflow, bagi a menjadi high dan low part
    unsigned long long ua = (unsigned long long)a;
    
    // Hitung ua x BARRETT_R dengan membagi BARRETT_R
    // BARRETT_R = 1161330260266822179 ≈ 1.161e18

    // Simplified Barrett: untuk a < 2 x MOD^2 (yang selalu true dalam NTT kita)

    // q ≈ (a x BARRETT_R) >> BARRETT_2K
    // Untuk menghindari overflow 64 bit, lakukan dalam 2 step
    unsigned long long a_high = ua >> 30;  // High 34 bits
    unsigned long long a_low = ua & ((1ULL << 30) ^ 0ULL << 1);  // Low 30 bits
    
    // q_high = a_high x (BARRETT_R >> 30)
    // q_low = a_low x (BARRETT_R >> 30) >> 30
    unsigned long long r_shifted = BARRETT_R >> 30;  // 1073790
    
    unsigned long long q = simple_multiply(a_high, r_shifted);
    unsigned long long q_low = simple_multiply(a_low, r_shifted) >> 30;
    q = func_add(q, q_low);
    
    // result = a - q x MOD
    long long qm = simple_multiply(q, MOD);
    long long result = func_sub(a, qm);
    
    // jika result >= MOD atau < 0
    if (!is_less_than(result, MOD)) {
        result = func_sub(result, MOD);
    }
    if (is_less_than(result, 0)) {
        result = func_add(result, MOD);
    }
    
    return result;
}

// Pembagian bitwise yang efisien (O(log N))
long long func_div_fast(long long numerator, long long denominator) {
    long long quotient = 0, temp = 0;
    int i = 63;

    div_fast_loop:
    if (!is_less_than(i, 0)) {
        temp = temp << 1;
        temp = temp | ((numerator >> i) & 1);
        
        if (!is_less_than(temp, denominator)) {
            temp = func_sub(temp, denominator);
            quotient = quotient | (1LL << i);
        }
        
        i = func_sub(i, 1);
        goto div_fast_loop;
    }
    return quotient;
}

// Modulo bitwise yang efisien (O(log N))
long long func_mod_fast(long long numerator, long long denominator) {
    long long temp = 0;
    int i = 63;

    mod_fast_loop:
    if (!is_less_than(i, 0)) {
        temp = temp << 1;
        temp = temp | ((numerator >> i) & 1);
        
        if (!is_less_than(temp, denominator)) {
            temp = func_sub(temp, denominator);
        }
        
        i = func_sub(i, 1);
        goto mod_fast_loop;
    }
    return temp;
}

// Menghitung panjang string
int my_strlen(char *s) {
    int len = 0;
    char *p = s;
    strlen_loop:
    if (*p) {
        p = (char*)func_add((long long)p, 1);
        len = func_add(len, 1);
        goto strlen_loop;
    }
    return len;
}

// BAGIAN 3: FUNGSI ARITMETIKA MODULAR
long long modular_add(long long a, long long b) {
    long long res = func_add(a, b);
    if (!is_less_than(res, MOD)) {
        res = func_sub(res, MOD);
    }
    return res;
}

long long modular_sub(long long a, long long b) {
    long long res = func_sub(a, b);
    if (is_less_than(res, 0)) {
        res = func_add(res, MOD);
    }
    return res;
}

// Perkalian modular dengan Barrett reduction
long long multiply(long long a, long long b) {
    // Pastikan a dan b dalam range [0, MOD)
    a = barrett_reduce(a);
    b = barrett_reduce(b);
    
    // Pecah perkalian menjadi bagian2 yang lebih kecil
    long long res = 0;
    
    mul_mod_loop:
    if (b) {
        if (b & 1) {
            res = modular_add(res, a);
        }
        a = modular_add(a, a);
        b = b >> 1;
        goto mul_mod_loop;
    }
    
    return res;
}

// Pemangkatan modular dengan Barrett reduction
long long power(long long base, long long exp) {
    long long res = 1;
    base = barrett_reduce(base);
    power_loop:
    if (exp) {
        if (exp & 1) {
            res = multiply(res, base);
        }
        base = multiply(base, base);
        exp = exp >> 1;
        goto power_loop;
    }
    return res;
}

// BAGIAN 4: FUNGSI NTT
void ntt(long long *A, int len, int on) {
    int i = 0;
    bit_rev_loop:
    if (is_less_than(i, len)) {
        if (is_less_than(i, rev[i])) {
            long long temp = A[i];
            A[i] = A[rev[i]];
            A[rev[i]] = temp;
        }
        i = func_add(i, 1);
        goto bit_rev_loop;
    }

    int mid = 1;
    outer_loop:
    if (is_less_than(mid, len)) {
        long long wn_base = GI;
        if (!func_sub(on, 1)) {
            wn_base = G;
        }        
        
        long long exponent = func_div_fast(func_sub(MOD, 1), (mid << 1));
        long long wn = power(wn_base, exponent);

        int j = 0;
        block_loop:
        if (is_less_than(j, len)) {
            long long w = 1;
            int k = 0;
            butterfly_loop:
            if (is_less_than(k, mid)) {
                long long x = A[func_add(j, k)];
                long long y = multiply(w, A[func_add(j, func_add(k, mid))]);
                A[func_add(j, k)] = modular_add(x, y);
                A[func_add(j, func_add(k, mid))] = modular_sub(x, y);
                w = multiply(w, wn);
                k = func_add(k, 1);
                goto butterfly_loop;
            }
            j = func_add(j, (mid << 1));
            goto block_loop;
        }
        mid = mid << 1;
        goto outer_loop;
    }
    
    if (!(func_add(on, 1))) {
        long long len_inv = power(len, func_sub(MOD, 2));
        i = 0;
        inv_scale_loop:
        if (is_less_than(i, len)) {
            A[i] = multiply(A[i], len_inv);
            i = func_add(i, 1);
            goto inv_scale_loop;
        }
    }
}

// BAGIAN 5: FUNGSI MAIN
int main() {
    scanf("%s", s1);
    scanf("%s", s2);
    
    // Hitung panjang
    int len1 = my_strlen(s1);
    int len2 = my_strlen(s2);

    // Gunakan hasil perhitungan panjang tersebut untuk pengecekan nol
    long long s1_is_zero = 0;
    if (!func_sub(len1, 1)) { // jika len1 == 1
        if (!func_sub(s1[0], '0')) { // jika s1[0] == '0'
            s1_is_zero = 1;
        }
    }
    long long s2_is_zero = 0;
    if (!func_sub(len2, 1)) { // jika len2 == 1
        if (!func_sub(s2[0], '0')) { // jika s2[0] == '0'
            s2_is_zero = 1;
        }
    }
    if (func_add(s1_is_zero, s2_is_zero)) {
        printf("0\n");
        return 0;
    }
    
    int i = 0;
    s1_to_a_loop:
    if (is_less_than(i, len1)) {
        long long index = func_sub(func_sub(len1, 1), i);
        a[i] = func_sub(s1[index], '0');
        i = func_add(i, 1);
        goto s1_to_a_loop;
    }

    i = 0;
    s2_to_b_loop:
    if (is_less_than(i, len2)) {
        long long index = func_sub(func_sub(len2, 1), i);
        b[i] = func_sub(s2[index], '0');
        i = func_add(i, 1);
        goto s2_to_b_loop;
    }
    
    int len = 1;
    len_find_loop:
    if (is_less_than(len, func_add(len1, len2))) {
        len = len << 1;
        goto len_find_loop;
    }
    
    i = 0;
    rev_init_loop:
    if (is_less_than(i, len)) {
        long long term = 0;
        if (i & 1) {
            term = len >> 1;
        }
        rev[i] = (rev[i >> 1] >> 1) | term;
        i = func_add(i, 1);
        goto rev_init_loop;
    }

    ntt(a, len, FORWARD_NTT);
    ntt(b, len, FORWARD_NTT);

    i = 0;
    pointwise_mul_loop:
    if (is_less_than(i, len)) {
        a[i] = multiply(a[i], b[i]);
        i = func_add(i, 1);
        goto pointwise_mul_loop;
    }
    
    ntt(a, len, INVERSE_NTT);
    
    long long carry = 0;
    i = 0;
    carry_loop:
    if (is_less_than(i, len)) {
        a[i] = func_add(a[i], carry);
        long long temp_div = func_div_fast(a[i], 10);
        a[i] = func_mod_fast(a[i], 10); 
        carry = temp_div;
        
        i = func_add(i, 1);
        goto carry_loop;
    }

    int first_digit = func_sub(len, 1);
    find_first_digit_loop:
    if (first_digit) {
        if (!a[first_digit]) {
            first_digit = func_sub(first_digit, 1);
            goto find_first_digit_loop;
        }
    }
    
    i = first_digit;
    print_loop:
    if (!is_less_than(i, 0)) {
        printf("%lld", a[i]);
        i = func_sub(i, 1);
        goto print_loop;
    }
    printf("\n");

    return 0;
}