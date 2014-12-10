// Example low level rendering Unity plugin


#include "UnityPluginInterface.h"
#include <math.h>
#include <stdio.h>
#include <tbb/tbb.h>
#include "MassParticle.h"
#include "mpTypes.h"

extern mpWorld *g_mpWorld;
extern mpRenderer *g_mpRenderer;


extern "C" EXPORT_API void mpGeneratePointMesh(int mi, mpMeshData *mds)
{
	g_mpWorld->generatePointMesh(mi, mds);
}

extern "C" EXPORT_API void mpGenerateCubeMesh(int mi, mpMeshData *mds)
{
	g_mpWorld->generateCubeMesh(mi, mds);
}

extern "C" EXPORT_API int mpUpdateDataTexture(mpWorld *context, void *tex)
{
    return context->updateDataTexture(tex);
}

#ifdef mpWithCppScript
extern "C" EXPORT_API int mpUpdateDataBuffer(mpWorld *context, UnityEngine::ComputeBuffer buf)
{
    return context->updateDataBuffer(buf);
}
struct mpUpdateDataBufferHelper
{
    mpUpdateDataBufferHelper()
    {
        cpsAddMethod("MPAPI::mpUpdateDataBuffer", &mpUpdateDataBuffer);
    }
} g_mpUpdateDataBufferHelper;

#endif // mpWithCppScript




extern "C" EXPORT_API mpWorld* mpCreateContext()
{
#ifdef mpWithCppScript
    cpsClearCache();
#endif // mpWithCppScript

    mpWorld *p = new mpWorld();
    return p;
}
extern "C" EXPORT_API void mpDestroyContext(mpWorld *c)
{
    delete c;
}
extern "C" EXPORT_API void mpMakeCurrent(mpWorld *c)
{
    g_mpWorld = c;
}


extern "C" EXPORT_API void mpBeginUpdate(float dt)
{
	g_mpWorld->beginUpdate(dt);
}
extern "C" EXPORT_API void mpEndUpdate()
{
	g_mpWorld->endUpdate();
}
extern "C" EXPORT_API void mpUpdate(float dt)
{
	g_mpWorld->update(dt);
}

extern "C" EXPORT_API void mpClearParticles()
{
	g_mpWorld->clearParticles();
}

extern "C" EXPORT_API void mpClearCollidersAndForces()
{
	g_mpWorld->clearCollidersAndForces();
}

extern "C" EXPORT_API ispc::KernelParams mpGetKernelParams()
{
	return g_mpWorld->getKernelParams();
}

extern "C" EXPORT_API void mpSetKernelParams(ispc::KernelParams *params)
{
	g_mpWorld->setKernelParams(*(mpKernelParams*)params);
}


extern "C" EXPORT_API int mpGetNumHitData()
{
	return g_mpWorld->getNumHitData();
}

extern "C" EXPORT_API mpHitData* mpGetHitData()
{
	return g_mpWorld->getHitData();
}

extern "C" EXPORT_API int mpGetNumParticles()
{
	return g_mpWorld->getNumParticles();
}
extern "C" EXPORT_API mpParticle* mpGetParticles()
{
	return g_mpWorld->getParticles();
}

extern "C" EXPORT_API void mpCopyParticles(mpParticle *dst)
{
	memcpy(dst, g_mpWorld->getParticles(), sizeof(mpParticle)*mpGetNumParticles());
}

extern "C" EXPORT_API void mpWriteParticles(const mpParticle *from)
{
	memcpy(g_mpWorld->getParticles(), from, sizeof(mpParticle)*mpGetNumParticles());
}


extern "C" EXPORT_API void mpScatterParticlesSphere(vec3 *center, float radius, int32_t num, vec3 *vel_base, float vel_diffuse)
{
	if (num <= 0) { return; }

	mpParticleCont particles(num);
	for (size_t i = 0; i < particles.size(); ++i) {
		float l = mpGenRand()*radius;
		vec3 dir = glm::normalize(vec3(mpGenRand(), mpGenRand(), mpGenRand()));
		vec3 pos = *center + dir*l;
		vec3 vel = vec3(
			vel_base->x + mpGenRand()*vel_diffuse,
			vel_base->y + mpGenRand()*vel_diffuse,
			vel_base->z + mpGenRand()*vel_diffuse );

		(vec3&)particles[i].velocity = vel;
		(vec3&)particles[i].position = pos;
	}
	g_mpWorld->addParticles(&particles[0], particles.size());
}

extern "C" EXPORT_API void mpScatterParticlesBox(vec3 *center, vec3 *size, int32_t num, vec3 *vel_base, float vel_diffuse)
{
	if (num <= 0) { return; }

	mpParticleCont particles(num);
	for (size_t i = 0; i < particles.size(); ++i) {
		vec3 pos = *center + vec3(mpGenRand()*size->x, mpGenRand()*size->y, mpGenRand()*size->z);
		vec3 vel = vec3(
			vel_base->x + mpGenRand()*vel_diffuse,
			vel_base->y + mpGenRand()*vel_diffuse,
			vel_base->z + mpGenRand()*vel_diffuse);

		(vec3&)particles[i].position = pos;
		(vec3&)particles[i].velocity = vel;
	}
	g_mpWorld->addParticles(&particles[0], particles.size());
}


extern "C" EXPORT_API void mpScatterParticlesSphereTransform(mat4 *transform, int32_t num, vec3 *vel_base, float vel_diffuse)
{
	if (num <= 0) { return; }

	mpParticleCont particles(num);
	simdmat4 mat(*transform);
	for (size_t i = 0; i < particles.size(); ++i) {
		vec3 dir = glm::normalize(vec3(mpGenRand(), mpGenRand(), mpGenRand()));
		float l = mpGenRand()*0.5f;
		simdvec4 pos = simdvec4(dir*l, 1.0f);
		pos = mat * pos;
		vec3 vel = vec3(
			vel_base->x + mpGenRand()*vel_diffuse,
			vel_base->y + mpGenRand()*vel_diffuse,
			vel_base->z + mpGenRand()*vel_diffuse);

		(vec3&)particles[i].position = (vec3&)pos;
		(vec3&)particles[i].velocity = vel;
	}
	g_mpWorld->addParticles(&particles[0], particles.size());
}

extern "C" EXPORT_API void mpScatterParticlesBoxTransform(mat4 *transform, int32_t num, vec3 *vel_base, float vel_diffuse)
{
	if (num <= 0) { return; }

	mpParticleCont particles(num);
	simdmat4 mat(*transform);
	for (size_t i = 0; i < particles.size(); ++i) {
		simdvec4 pos(mpGenRand()*0.5f, mpGenRand()*0.5f, mpGenRand()*0.5f, 1.0f);
		pos = mat * pos;
		vec3 vel = vec3(
			vel_base->x + mpGenRand()*vel_diffuse,
			vel_base->y + mpGenRand()*vel_diffuse,
			vel_base->z + mpGenRand()*vel_diffuse);

		(vec3&)particles[i].position = (vec3&)pos;
		(vec3&)particles[i].velocity = vel;
	}
	g_mpWorld->addParticles(&particles[0], particles.size());
}



inline void mpBuildBoxCollider(mpBoxCollider &o, mat4 transform, vec3 size)
{
	float psize = g_mpWorld->getKernelParams().particle_size;
	size.x = size.x * 0.5f;
	size.y = size.y * 0.5f;
	size.z = size.z * 0.5f;

	simdmat4 st = simdmat4(transform);
	simdvec4 vertices[] = {
		simdvec4(size.x, size.y, size.z, 0.0f),
		simdvec4(-size.x, size.y, size.z, 0.0f),
		simdvec4(-size.x, -size.y, size.z, 0.0f),
		simdvec4(size.x, -size.y, size.z, 0.0f),
		simdvec4(size.x, size.y, -size.z, 0.0f),
		simdvec4(-size.x, size.y, -size.z, 0.0f),
		simdvec4(-size.x, -size.y, -size.z, 0.0f),
		simdvec4(size.x, -size.y, -size.z, 0.0f),
	};
	for (int i = 0; i < mpCountof(vertices); ++i) {
		vertices[i] = st * vertices[i];
	}

	simdvec4 normals[6] = {
		glm::normalize(glm::cross(vertices[3] - vertices[0], vertices[4] - vertices[0])),
		glm::normalize(glm::cross(vertices[5] - vertices[1], vertices[2] - vertices[1])),
		glm::normalize(glm::cross(vertices[7] - vertices[3], vertices[2] - vertices[3])),
		glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[4] - vertices[0])),
		glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[3] - vertices[0])),
		glm::normalize(glm::cross(vertices[7] - vertices[4], vertices[5] - vertices[4])),
	};
	float distances[6] = {
		-(glm::dot(vertices[0], normals[0]) + psize),
		-(glm::dot(vertices[1], normals[1]) + psize),
		-(glm::dot(vertices[0], normals[2]) + psize),
		-(glm::dot(vertices[3], normals[3]) + psize),
		-(glm::dot(vertices[0], normals[4]) + psize),
		-(glm::dot(vertices[4], normals[5]) + psize),
	};

	(vec3&)o.shape.center = (vec3&)transform[3][0];
	for (int i = 0; i < 6; ++i) {
		(vec3&)o.shape.planes[i].normal = (vec3&)normals[i];
		o.shape.planes[i].distance = distances[i];
	}
	for (int i = 0; i < mpCountof(vertices); ++i) {
		vec3 p = (vec3&)vertices[i];
		if (i == 0) {
			(vec3&)o.bounds.bl = p;
			(vec3&)o.bounds.ur = p;
		}
		(vec3&)o.bounds.bl = glm::min((vec3&)o.bounds.bl, p - psize);
		(vec3&)o.bounds.ur = glm::max((vec3&)o.bounds.ur, p + psize);
	}
}

inline void mpBuildSphereCollider(mpSphereCollider &o, vec3 center, float radius)
{
	float psize = g_mpWorld->getKernelParams().particle_size;
	float er = radius + psize;
	(vec3&)o.shape.center = center;
	o.shape.radius = er;
	(vec3&)o.bounds.bl = center - er;
	(vec3&)o.bounds.ur = center + er;
}

inline void mpBuildCapsuleCollider(mpCapsuleCollider &o, vec3 pos1, vec3 pos2, float radius)
{
	float psize = g_mpWorld->getKernelParams().particle_size;
	float er = radius + psize;

	ispc::Capsule &shape = o.shape;
	(vec3&)shape.pos1 = pos1;
	(vec3&)shape.pos2 = pos2;
	shape.radius = er;
	float len_sq = glm::length_sq((vec3&)shape.pos2 - (vec3&)shape.pos1);
	shape.rcp_lensq = 1.0f / len_sq;

	(vec3&)o.bounds.bl = glm::min((vec3&)shape.pos1 - er, (vec3&)(shape.pos2) - er);
	(vec3&)o.bounds.ur = glm::max((vec3&)shape.pos1 + er, (vec3&)(shape.pos2) + er);
}


extern "C" EXPORT_API void mpAddBoxCollider(mpColliderProperties *props, mat4 *transform, vec3 *size)
{
	mpBoxCollider col;
	col.props = *props;
	mpBuildBoxCollider(col, *transform, *size);
	g_mpWorld->addBoxColliders(&col, 1);
}

extern "C" EXPORT_API void mpAddSphereCollider(mpColliderProperties *props, vec3 *center, float radius)
{
	mpSphereCollider col;
	col.props = *props;
	mpBuildSphereCollider(col, *center, radius);
	g_mpWorld->addSphereColliders(&col, 1);
}

extern "C" EXPORT_API void mpAddCapsuleCollider(mpColliderProperties *props, vec3 *pos1, vec3 *pos2, float radius)
{
	mpCapsuleCollider col;
	col.props = *props;
	mpBuildCapsuleCollider(col, *pos1, *pos2, radius);
	g_mpWorld->addCapsuleColliders(&col, 1);
}

extern "C" EXPORT_API void mpAddForce(mpForceProperties *props, mat4 *_trans)
{
    mat4 &trans = *_trans;
	mpForce force;
	force.props = *props;
	force.props.rcp_range = 1.0f / (force.props.range_outer - force.props.range_inner);

	switch (force.props.shape_type) {
	case mpFS_Sphere:
		{
			mpSphereCollider col;
			vec3 pos = (vec3&)trans[3];
			float radius = (trans[0][0] + trans[1][1] + trans[2][2]) * 0.3333333333f * 0.5f;
			mpBuildSphereCollider(col, pos, radius);
			force.bounds = col.bounds;
			force.sphere.center = col.shape.center;
			force.sphere.radius = col.shape.radius;
		}
		break;

	case mpFS_Capsule:
		{
			mpCapsuleCollider col;
			vec3 pos = (vec3&)trans[3];
			float radius = (trans[0][0] + trans[2][2]) * 0.5f * 0.5f;
			mpBuildCapsuleCollider(col, pos, pos, radius);
			force.bounds = col.bounds;
			force.sphere.center = col.shape.center;
			force.sphere.radius = col.shape.radius;
		}
		break;

	case mpFS_Box:
		{
			mpBoxCollider col;
			mpBuildBoxCollider(col, trans, vec3(1.0f, 1.0f, 1.0f));
			force.bounds = col.bounds;
			force.box.center = col.shape.center;
			for (int i = 0; i < 6; ++i) {
				force.box.planes[i] = col.shape.planes[i];
			}
		}
		break;

	}
	g_mpWorld->addForces(&force, 1);
}

