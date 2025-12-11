
#include "Shared.hlsli"

struct Particle
{
	float3 position;
	float lifeTime;
	
	float3 velocity;
	float elapsedTime;
	
	float3 acceleration;
	float pad;
	
	float3x3 rotation;
};

struct ParticleOption
{
	int rotationIndex;
	float padding[3];
};

cbuffer ParticleOptionBuffer : register(b4)
{
	ParticleOption option;
};

StructuredBuffer<Particle> addParticles : register(t0);

ConsumeStructuredBuffer<Particle> originParticles : register(u0);
AppendStructuredBuffer<Particle> activeParticles : register(u1);
AppendStructuredBuffer<Particle> deadParticles : register(u2);




[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	Particle particle = originParticles.Consume();
	particle.velocity += particle.acceleration * frameData.deltaTime;
	particle.position += particle.velocity * frameData.deltaTime;
	particle.lifeTime -= frameData.deltaTime;
	
	particle.elapsedTime += frameData.deltaTime;
	
	switch (option.rotationIndex)
	{
		case 0:
			particle.rotation = (float3x3) IVM;
			break;
		case 1:
			float3 T = normalize(particle.velocity);
		
			float3 camUp = normalize(float3(IVM[1][0], IVM[1][1], IVM[1][2]));
		
			float refDot = abs(dot(T, camUp));
			float3 refVec;
			if (refDot < 0.5)
			{
				refVec = normalize(float3(IVM[0][0], IVM[0][1], IVM[0][2]));
			}
			else
			{
				refVec = camUp;
			}
 

			float3 B = normalize(cross(refVec, T));
			float3 N = cross(T, B);
			float3x3 TBN = float3x3(T, B, N);
			particle.rotation = TBN;
			break;
		case 2:
			particle.rotation = particle.rotation;
			break;
	}


	
	[branch]
	if (particle.lifeTime > 0)
	{
		activeParticles.Append(particle);
	}
	else if (particle.elapsedTime != frameData.deltaTime)
	{
		deadParticles.Append(particle);
	}
	
	uint size, stride;
	addParticles.GetDimensions(size, stride);
	if (DTid.x < size)
	{
		Particle addParticle = addParticles[DTid.x];
		
		if (addParticle.lifeTime > 0)
		{
			activeParticles.Append(addParticle);
		}
	}
	
	
}
