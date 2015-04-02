float ComputeDepth(float4 clippos)
{
#if defined(SHADER_API_OPENGL) || defined(SHADER_API_GLES)
	return ((clippos.z / clippos.w) + 1.0) * 0.5;
#else
	return clippos.z / clippos.w;
#endif
}

