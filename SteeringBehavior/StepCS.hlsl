#include"Common.hlsli"

cbuffer StepParameters : register(b2)
{
    uint positionVelocityWriteIndex;
    
    uint positionVelocityReadIndex;
    
    uint maxSpeedMaxForceIndex;
    
    uint numVehicle;
    
    float speedMultiply;
    
    float2 mousePos;
    
    float fleeRadius;
}

static RWBuffer<float4> positionVelocityWrite = ResourceDescriptorHeap[positionVelocityWriteIndex];

static Buffer<float4> positionVelocityRead = ResourceDescriptorHeap[positionVelocityReadIndex];

static Buffer<float2> maxSpeedMaxForce = ResourceDescriptorHeap[maxSpeedMaxForceIndex];

static float2 position;

static float2 velocity;

static float mSpeed;

static float mForce;

void limitMag(inout float2 v, float maxMag)
{
    float mag = length(v);

    mag = clamp(mag, 0.0, maxMag);
    
    v = normalize(v) * mag;
}

void setMag(inout float2 v, float mag)
{
    v = normalize(v) * mag;
}

float2 seek(float2 target)
{
    float2 force = target - position;
    setMag(force, mSpeed);
    force -= velocity;
    limitMag(force, mForce);
    return force;
}

float2 flee(float2 target)
{
    float2 force = position - target;
    setMag(force, mSpeed);
    force -= velocity;
    limitMag(force, mForce);
    return force;
}

float2 separate()
{
    float desiredSeparation = 25.f;
    
    float2 steer = float2(0, 0);
    
    uint count = 0;
    
    for (uint i = 0; i < numVehicle; i++)
    {
        float d = length(position - positionVelocityRead[i].xy);
        if ((d > 0) && (d < desiredSeparation))
        {
            float2 diff = normalize(position - positionVelocityRead[i].xy) / d;
            steer += diff;
            count++;
        }
    }
    
    if (count > 0)
        steer /= float(count);
    
    if (length(steer) > 0)
    {
        setMag(steer, mSpeed);
        steer -= velocity;
        limitMag(steer, mForce);
    }
    
    return steer;
}

float2 align()
{
    float neighborDist = 50.f;
    
    float2 sum = float2(0, 0);
    
    uint count = 0;
    
    for (uint i = 0; i < numVehicle; i++)
    {
        float d = length(position - positionVelocityRead[i].xy);
        if ((d > 0) && (d < neighborDist))
        {
            sum += positionVelocityRead[i].zw;
            count++;
        }
    }

    if (count > 0)
    {
        setMag(sum, mSpeed);
        float2 steer = sum - velocity;
        limitMag(steer, mForce);
        return steer;
    }
    else
    {
        return float2(0, 0);
    }
}

float2 cohesion()
{
    float neighborDist = 50.f;
    
    float2 sum = float2(0, 0);
    
    uint count = 0;
    
    for (uint i = 0; i < numVehicle; i++)
    {
        float d = length(position - positionVelocityRead[i].xy);
        if ((d > 0) && (d < neighborDist))
        {
            sum += positionVelocityRead[i].xy;
            count++;
        }
    }

    if (count > 0)
    {
        sum /= float(count);
        return seek(sum);
    }
    else
    {
        return float2(0, 0);
    }
}

[numthreads(32, 1, 1)]
void main(uint DTid : SV_DispatchThreadID)
{
    position = positionVelocityRead[DTid].xy;
    
    velocity = positionVelocityRead[DTid].zw;
    
    mSpeed = maxSpeedMaxForce[DTid].x;
    
    mForce = maxSpeedMaxForce[DTid].y;
    
    float2 sep = separate() * 2.0;
    
    float2 ali = align();
    
    float2 coh = cohesion();
    
    float2 acceleration = sep + ali + coh;
    
    if (distance(position, mousePos) < fleeRadius)
    {
        acceleration += flee(mousePos) * 3.0;
    }
    
    float scaledDt = perframeResource.deltaTime * speedMultiply;
    
    velocity += acceleration * scaledDt;
    
    limitMag(velocity, mSpeed);
    
    position += velocity * scaledDt;
    
    if (position.x < -40)
    {
        position.x = 1960;
    }
    else if (position.x > 1960)
    {
        position.x = -40;
    }
    
    if (position.y < -40)
    {
        position.y = 1120;
    }
    else if (position.y > 1120)
    {
        position.y = -40;
    }
    
    positionVelocityWrite[DTid] = float4(position, velocity);
}