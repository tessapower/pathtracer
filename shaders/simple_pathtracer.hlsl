// Simple gradient compute shader for testing
RWTexture2D<float4> OutputTexture : register(u0);

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    OutputTexture.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height)
        return;

    // Generate gradient pattern
    float r = (float)DTid.x / (float)width;
    float g = (float)DTid.y / (float)height;
    float b = 0.5;

    OutputTexture[DTid.xy] = float4(r, g, b, 1.0);
}
