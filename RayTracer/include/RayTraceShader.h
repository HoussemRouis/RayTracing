#include <glad/glad.h>

#ifndef STRINGIFY
#define STRINGIFY(A)  #A
#endif // 


static const GLchar* rayTraceCS = STRINGIFY(
\n#version 430 core\n

layout(binding = 0, rgba32f) uniform image2D framebuffer;


\n#define reflectionMaxDepth 100\n
\n#define lightsMaxNbr 10\n
\n#define objectsMaxNbr 20\n

struct Light {
	vec3 pos;
	vec4 color;
};

struct Object {
	float type;
	vec3 pos;
	float r;
	vec3 min;
	vec3 max;
	vec4 color;
};

uniform vec3 eye;
uniform mat4 view;
uniform mat4 inversinvProjectionView;
uniform float dnear;
uniform float dfar;
uniform int depthMax;
uniform int lightsNbr;
uniform int objectsNbr;
uniform Light vLights[lightsMaxNbr];
uniform Object vObjects[objectsMaxNbr];
uniform vec4 emission;
uniform vec4 reflection;

struct Ray {
	vec3 origin;
	vec3 dir;
};


struct hitInfo {
	float distFromCam;
	int objIdx;
	vec3 normalAtPt;
};


//Returns the distances from the origin of the ray to the closest hit and outputs the normal
float boxIntersect(Ray ray, const vec3 minCorner, const vec3 maxCorner, out vec3 outNormal) {

	vec3 tMin = (minCorner - ray.origin) / ray.dir;
	vec3 tMax = (maxCorner - ray.origin) / ray.dir;
	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);

	float tN = max(max(t1.x, t1.y), t1.z);
	float tF = min(min(t2.x, t2.y), t2.z);
	outNormal = -sign(ray.dir)*step(t1.yzx, t1.xyz)*step(t1.zxy, t1.xyz);

	if (tN > tF) return -1.0f; // no intersection
	else return tN;

}

//Returns the distances from the origin of the ray to the closest hit and outputs the normal
float sphereIntersect(Ray ray, vec3 center, float radius, out  vec3 outNormal)
{

	vec3 oc = ray.origin - center;
	float b = dot(oc, ray.dir);
	float c = dot(oc, oc) - radius * radius;
	float h = b * b - c;
	if (h < 0.0f) return -1.0f; // no intersection
	h = sqrt(h);
	float dist = -b - h;

	vec3 nrml = (1.0f / radius)*(ray.origin + dist * ray.dir - center);
	float nrml_norm = dot(nrml, nrml);
	outNormal = nrml / nrml_norm;

	return dist;
}


//returns wether an object is hit along the ray and stocks the results in the hitInfo
bool intersectObjects(Ray ray, out hitInfo info) {

	//start the furthest point in the frustum
	float closest = dfar;
	bool found = false;

	for (int i = 0; i < objectsNbr; i++) {
		float distFromCam;
		vec3 normalAtPt;
		if (vObjects[i].type == 0.0f)
		{
			distFromCam = sphereIntersect(ray, vObjects[i].pos, vObjects[i].r, normalAtPt);

		}
		else
		{
			distFromCam = boxIntersect(ray, vObjects[i].min, vObjects[i].max, normalAtPt);

		}
		//set up the intersection with the closest hit
		if (distFromCam > 0.0f && distFromCam < closest) {
			closest = distFromCam;
			info.distFromCam = 0.99f * distFromCam;
			info.objIdx = i;
			info.normalAtPt = normalAtPt;
			found = true;

		}
	}
	return found;
}

//Apply lighting to the objects
vec4 computeLighting(vec3 intersectionPt, vec3 normalAtPt, int objIdx)
{
	vec4 iL = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	// Go though all light sources to update texel colors
	for (int l = 0; l < lightsNbr; l++)
	{
		hitInfo j;

		vec3 shadowRayDir = normalize(vLights[l].pos - intersectionPt);
		Ray shadowRay;
		shadowRay.origin = intersectionPt;
		shadowRay.dir = shadowRayDir;
		//if no object was found, we set the color, we set to shadow color otherwise
		if (!intersectObjects(shadowRay, j))
		{
			float light_cos = dot(normalAtPt, shadowRayDir);
			iL += light_cos * vObjects[objIdx].color*vLights[l].color;

		}

	}
	return iL;
}

bool reflectRay(Ray ray,out vec4 iL, out vec4 reflection,out hitInfo j)
{
	iL = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	if (intersectObjects(ray, j)) {
		reflection = reflection;
		vec3 intersectionPt = ray.origin + ray.dir * j.distFromCam;
		iL += computeLighting(intersectionPt, j.normalAtPt, j.objIdx);
		return true;
	}
	return false;
}


vec4 traceRay(vec3 origin, vec3 dir) {
	Ray currentRay;
	currentRay.origin = origin;
	currentRay.dir = dir;
	
	vec4 iR = vec4(0.0f, 0.0f, 0.0f, 1.0f);	//Reflection Term
	vec4 iE = vec4(0.0f, 0.0f, 0.0f, 1.0f); //Emission Term

	//Used to stock reflection info throughout raycasting
	vec4 iL_vect[reflectionMaxDepth];
	vec4 reflection_vect[reflectionMaxDepth];
	hitInfo i;
	//Do the first ray casting
	if (intersectObjects(currentRay, i)) {
		iE += emission;
		i.distFromCam = i.distFromCam;

		vec3 intersectionPt = currentRay.origin + currentRay.dir * i.distFromCam;

		iL_vect[0] = computeLighting(intersectionPt, i.normalAtPt, i.objIdx);
		reflection_vect[0] = reflection;
		
		//updating the ray
		currentRay.origin = currentRay.origin + currentRay.dir * i.distFromCam;
		float n_dot_dir = dot(i.normalAtPt, currentRay.dir);
		currentRay.dir = currentRay.dir - 2.0f * n_dot_dir*i.normalAtPt;
		//currentRay.dir = reflect(currentRay.dir, i.normalAtPt);	

		//Reflect the ray depthMax-1 times
		for (int depth = 1; depth < depthMax; depth++)
		{
			hitInfo j;
			//No need to go through the rest of the iterations if we dont hit and object
			if (!reflectRay(currentRay, iL_vect[depth], reflection_vect[depth], j))
				break;

			//updating the ray
			currentRay.origin = currentRay.origin + currentRay.dir * j.distFromCam;
			float n_dot_dir = dot(j.normalAtPt, currentRay.dir);
			currentRay.dir = currentRay.dir - 2.0f * n_dot_dir*j.normalAtPt;
			//currentRay.dir = reflect(currentRay.dir, j.normalAtPt);
		}

		//iReflect = iL + R*( iL' + R'*( iL" + ...))
		for (int d = depthMax - 1; d >= 0; d--)
		{
			vec4 tmp = iR;
			iR = iL_vect[d] + reflection_vect[d] * tmp;
		}

	}


	return iR + iE;
}


layout(local_size_x = 8, local_size_y = 8) in;

void main(void)
{

	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
	ivec2 frameSize = imageSize(framebuffer);
	if (texel.x >= frameSize.x || texel.y >= frameSize.y) {
		return;
	}
	vec2 pos = vec2(texel) / vec2(frameSize.x, frameSize.y);


	vec2 texCoord = vec2(float(texel.x) / float(frameSize.x),
		float(texel.y) / float(frameSize.y));

	//Normalized coordinates
	vec2 nCoords = (2.0f * texCoord - 1.0f);


	//Setting up the ray from camera to the texel
	float frustumDepth = dfar - dnear;
	float frustumSum = dfar + dnear;
	vec4 camRay = inversinvProjectionView * vec4(nCoords * frustumDepth, frustumSum, frustumDepth);
	vec3 dir = normalize(camRay).xyz;


	vec4 color = traceRay(eye, dir);

	imageStore(framebuffer, texel, clamp(color, 0.0f, 1.0f));
}
);