// Constant camera buffer
cbuffer CameraData : register(b0)
{
    float3 cameraPosition;
    float fovTanHalf;

    float3 cameraRight;
    float aspectRatio;

    float3 cameraUp;
    float _pad0;

    float3 cameraForward;
    float _pad1;
};

RWTexture2D<float4> OutputTexture : register(u0);

// Simple sphere intersection
bool intersectSphere(float3 rayOrigin, float3 rayDirection, float3 sphereCenter, float sphereRadius, out float t)
{
    t = 0.0;
    
    // Origin to center vector
    float3 oc = rayOrigin - sphereCenter;
    
    // Quadratic coefficients
    float a = dot(rayDirection, rayDirection);
    float h = dot(rayDirection, oc);
    float c = dot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = h * h - a * c;

    if (discriminant < 0)
        return false;

    float sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    float root = (-h - sqrtd) / a;

    if (root <= 0.0)
    {
        root = (-h + sqrtd) / a;
        if (root <= 0.0)
            return false;
    }

    t = root;

    return true;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    OutputTexture.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height)
        return;

    // Compute UV in [-1, 1] range
    float2 uv = (float2(DTid.xy) + 0.5) / float2(width, height);
    uv = uv * 2.0 - 1.0;
    uv.y = -uv.y; // Flip Y for correct orientation

    // Generate ray from camera
    uv.x *= aspectRatio;
    uv *= fovTanHalf;
    float3 rayDirection
        = normalize(cameraForward + uv.x * cameraRight + uv.y * cameraUp);
    float3 rayOrigin = cameraPosition;

    // Test scene: sphere at origin
    float3 sphereCenter = float3(0, 0, 0);
    float sphereRadius = 1.0;
    
    float t = 0.0;
    float3 color = float3(0, 0, 0);

    if (intersectSphere(rayOrigin, rayDirection, sphereCenter, sphereRadius, t))
    {
        // Hit sphere, shade like normal
        float3 hitPoint = rayOrigin + rayDirection * t;
        float3 normal = normalize(hitPoint - sphereCenter);
        // Simple normal-based color, map [-1, 1] to [0, 1] range
        color = normal * 0.5 + 0.5;
    }
    else
    {
        // Sky color
        float gradient = uv.y * 0.5 + 0.5; // Vertical gradient
        color = lerp(float3(1.0, 1.0, 1.0), float3(0.5, 0.7, 1.0), gradient);
    }

        OutputTexture[DTid.xy] = float4(color, 1.0);
}
