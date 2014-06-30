#!/usr/bin/ruby


CFLAGS="-I ../external/glm -I ../external/tbb/include -I ./ -std=c++11 -stdlib=libc++"
LFLAGS="-L ../external/tbb/lib/intel64 -lc -lc++ -ltbb -framework OpenGL -demangle -dynamic -arch x86_64"
CC="clang++"

`mkdir -p MassParticle`
`mkdir -p MassParticleHelper`
`./ispc ../mpCore.ispc -o MassParticle/mpCore.o -h mpCore_ispc.h --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math`

[
  "../UnityPluginInterface.cpp",
  "../MassParticle.cpp",
  "../mpRendererOpenGL.cpp",
  "../mpTypes.cpp",
  "../tasksys.cpp",
].each do |cpp|
  `#{CC} -c #{cpp} #{CFLAGS} -o MassParticle/#{cpp.gsub(/.+?\//,'').gsub(/\.cpp/, ".o")}`
end
`ld -dylib MassParticle/*.o #{LFLAGS} -o MassParticle.dylib`


MPHSources = [
  "../MassParticleHelper.cpp"
].each do |cpp|
  `#{CC} -c #{cpp} #{CFLAGS} -o MassParticleHelper/#{cpp.gsub(/.+?\//,"").gsub(/\.cpp/, ".o")}`
end
`ld -dylib MassParticleHelper/*.o #{LFLAGS} -o MassParticleHelper.dylib`
