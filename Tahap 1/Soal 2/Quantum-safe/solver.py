from fractions import Fraction
import re

# CONFIG
P = [[47, -77, -85],[-49, 78, 50],[57, -78, 99]]

KNOWN_FIRST_CHAR = 'H'
KNOWN_FIRST_ORD = ord(KNOWN_FIRST_CHAR)

ciphertexts = []

with open("enc.txt", "r") as f:
    content = f.read().strip()
    content = content.replace("(", "").replace(")", "")
    parts = content.split(",")
    nums = [int(x.strip()) for x in parts if x.strip() != ""]
    for i in range(0, len(nums), 3):
        ciphertexts.append(tuple(nums[i:i+3]))

# linear algebra helper (rational inverse)
def det3(m):
    a,b,c = m[0]
    d,e,f = m[1]
    g,h,i = m[2]
    return (a*e*i + b*f*g + c*d*h) - (c*e*g + b*d*i + a*f*h)

def adjugate3(m):
    a,b,c = m[0]
    d,e,f = m[1]
    g,h,i = m[2]
    # cofactor matrix transposed (adjugate)
    return [
        [ (e*i - f*h), -(b*i - c*h),  (b*f - c*e) ],
        [-(d*i - f*g),  (a*i - c*g), -(a*f - c*d) ],
        [ (d*h - e*g), -(a*h - b*g),  (a*e - b*d) ]
    ]

def inv_matrix_rational(m):
    D = det3(m)
    if D == 0:
        raise ValueError("Matrix singular, tidak invertible")
    adj = adjugate3(m)
    # produce matrix of Fractions
    return [[ Fraction(adj[i][j], D) for j in range(3)] for i in range(3)]

def vec_sub(a,b):
    return [a[i]-b[i] for i in range(3)]

def vec_mul_mat_rational(v, mat_rational):
    # v is length 3 (ints), mat_rational is 3x3 of Fraction, returns list of Fractions
    res = []
    for j in range(3):
        s = Fraction(0,1)
        for k in range(3):
            s += Fraction(v[k], 1) * mat_rational[k][j]
        res.append(s)
    return res

# decryption routine
def recover_flag(ciphertexts, P, known_first_ord):
    if len(ciphertexts) == 0:
        return ""
    P_inv = inv_matrix_rational(P)
    c0 = ciphertexts[0]
    flag_chars = []
    for cj in ciphertexts:
        delta = vec_mul_mat_rational(vec_sub(cj, c0), P_inv)  # this is p_j - p_0
        # first element of p_j = known_first_ord + delta[0]
        # delta[0] should be integer (Fraction with denominator 1)
        if delta[0].denominator != 1:
            val0 = int(delta[0])
        else:
            val0 = delta[0].numerator
        ascii_code = known_first_ord + val0
        if not (0 <= ascii_code <= 0x10FFFF):
            raise ValueError(f"ascii out of range: {ascii_code} (delta={delta[0]})")
        flag_chars.append(chr(ascii_code))
    return "".join(flag_chars)

# helper: parse lines like "Vector([x, y, z])"
def parse_printed_vectors(text):
    # cari semua triplet angka (negatif atau positif)
    nums = re.findall(r'-?\d+', text)
    triples = []
    for i in range(0, len(nums), 3):
        if i+2 < len(nums):
            triples.append([int(nums[i]), int(nums[i+1]), int(nums[i+2])])
    return triples

# ---------- main ----------
if __name__ == "__main__":
    flag = recover_flag(ciphertexts, P, KNOWN_FIRST_ORD)
    print(flag)