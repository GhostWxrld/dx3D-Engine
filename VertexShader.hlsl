
cbuffer CBuf {
     matrix transform;                                           //The word matrix means float4x4
};

float4 main(float3 pos : Position) : SV_Position{
    
    return mul(float4(pos, 1.0f), transform);
}   