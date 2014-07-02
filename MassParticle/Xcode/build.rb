#!/usr/bin/ruby



`mkdir -p x86`
`mkdir -p x86_64`
`mkdir -p MassParticleHelper`
`./ispc ../mpCore.ispc -o x86/mpCore.o -h mpCore_ispc.h --target=sse2,sse4,avx --arch=x86 --opt=fast-masked-vload --opt=fast-math`
`./ispc ../mpCore.ispc -o x86_64/mpCore.o -h mpCore_ispc.h --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math`
