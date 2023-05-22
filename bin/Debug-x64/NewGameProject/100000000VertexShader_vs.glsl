#if 0
	***	hlms_uv_count0	2
	***	hlms_pose	0
	***	uv_emissive	0
	***	first_valid_detail_map_nm	4
	***	fresnel_scalar	0
	***	hlms_uv_count	1
	***	uv_detail_nm1	0
	***	uv_detail_nm2	0
	***	areaLightMasks	1
	***	precision_mode	-2126167738
	***	BRDF_Default	1
	***	set0_texture_slot_end	3
	***	uv_specular	0
	***	hlms_lights_area_approx	4
	***	hlms_forwardplus_flipY	1
	***	glsl	635204550
	***	hlms_lights_directional	0
	***	samplerStateStart	3
	***	alpha_test	0
	***	GGX_height_correlated	1
	***	glsles	1070293233
	***	metallic_workflow	0
	***	hw_gamma_write	1
	***	uv_detail_weight	0
	***	hlms_lights_area_tex_colour	1
	***	hlms_alpha_to_coverage	0
	***	cubemaps_as_diffuse_gi	1
	***	uv_detail_nm3	0
	***	hlms_pose_normals	0
	***	hlms_pose_half	0
	***	hlms_lights_spot	1
	***	GL_ARB_shading_language_420pack	1
	***	uv_normal	0
	***	relaxed	1726237731
	***	hlms_alphablend	0
	***	hlslvk	1841745752
	***	uv_detail3	0
	***	normal_map	0
	***	hlms_bones_per_vertex	3
	***	uv_detail1	0
	***	fresnel_workflow	0
	***	full32	-2126167738
	***	perceptual_roughness	1
	***	alpha_test_shadow_caster_only	0
	***	materials_per_buffer	240
	***	hw_gamma_read	1
	***	hlms_skeleton	1
	***	midf16	-1978079318
	***	syntax	-338983575
	***	hlms_lights_point	1
	***	metal	-1698855755
	***	GL_ARB_base_instance	1
	***	s_lights_directional_non_caster	1
	***	uv_detail0	0
	***	uv_diffuse	0
	***	uv_detail_nm0	0
	***	GL3+	450
	***	hlms_render_depth_only	0
	***	num_pass_const_buffers	3
	***	uv_detail2	0
	***	uv_roughness	0
	***	receive_shadows	1
	***	hlms_lights_area_tex_mask	1
	***	clear_coat	0
	***	needs_view_dir	1
	***	set1_texture_slot_end	3
	***	use_planar_reflections	0
	***	hlms_normal	1
	***	ltc_texture_available	1
	***	glslvk	-338983575
	***	hlsl	-334286542
	***	normal_weight	0
	***	GL_ARB_texture_buffer_range	1
	DONE DUMPING PROPERTIES
	DONE DUMPING PIECES
#endif


	#version 450 core







	#extension GL_ARB_shading_language_420pack: require
	#define layout_constbuffer(x) layout( std140, x )





	#define bufferFetch texelFetch
	#define structuredBufferFetch texelFetch



	#define min3( a, b, c ) min( a, min( b, c ) )
	#define max3( a, b, c ) max( a, max( b, c ) )


#define float2 vec2
#define float3 vec3
#define float4 vec4

#define int2 ivec2
#define int3 ivec3
#define int4 ivec4

#define uint2 uvec2
#define uint3 uvec3
#define uint4 uvec4

#define float2x2 mat2
#define float3x3 mat3
#define float4x4 mat4
#define ogre_float4x3 mat3x4

#define ushort uint
#define ushort3 uint3
#define ushort4 uint4

//Short used for read operations. It's an int in GLSL & HLSL. An ushort in Metal
#define rshort int
#define rshort2 int2
#define rint int
//Short used for write operations. It's an int in GLSL. An ushort in HLSL & Metal
#define wshort2 int2
#define wshort3 int3

#define toFloat3x3( x ) mat3( x )
#define buildFloat3x3( row0, row1, row2 ) mat3( row0, row1, row2 )

// Let's explain this madness:
//
// We use the keyword "midf" because "half" is already taken on Metal.
//
// When precision_mode == full32 midf is float. Nothing weird
//
// When precision_mode == midf16, midf and midf_c map both to float16_t. It's similar to full32
// but literals need to be prefixed with _h()
//
// Thus, what happens if we resolve some of the macros, we end up with:
//		float16_t a = 1.0f;						// Error
//		float16_t b = _h( 1.0f );				// OK!
//		float16_t c = float16_t( someFloat );	// OK!
//
// But when precision_mode == relaxed; we have the following problem:
//		mediump float a = 1.0f;							// Error
//		mediump float b = _h( 1.0f );					// OK!
//		mediump float c = mediump float( someFloat );	// Invalid syntax!
//
// That's where 'midf_c' comes into play. The "_c" means cast or construct. Hence we do instead:
//		midf c = midf( someFloat );		// Will turn into invalid syntax on relaxed!
//		midf c = midf_c( someFloat );	// OK!
//
// Therefore datatypes are declared with midf. And casts and constructors are with midf_c
// Proper usage is as follows:
//		midf b = _h( 1.0f );
//		midf b = midf_c( someFloat );
//		midf c = midf3_c( 1.0f, 2.0f, 3.0f );
//
// Using this convention ensures that code will compile with all 3 precision modes.
// Breaking this convention means one or more of the modes (except full32) will not compile.

	#define _h(x) (x)

	#define midf float
	#define midf2 vec2
	#define midf3 vec3
	#define midf4 vec4
	#define midf2x2 mat2
	#define midf3x3 mat3
	#define midf4x4 mat4

	#define midf_c float
	#define midf2_c vec2
	#define midf3_c vec3
	#define midf4_c vec4
	#define midf2x2_c mat2
	#define midf3x3_c mat3
	#define midf4x4_c mat4

	#define toMidf3x3( x ) mat3( x )
	#define buildMidf3x3( row0, row1, row2 ) mat3( row0, row1, row2 )

	#define ensureValidRangeF16(x)

	#define saturate(x) clamp( (x), 0.0, 1.0 )




#define mul( x, y ) ((x) * (y))
#define lerp mix
#define rsqrt inversesqrt
#define INLINE
#define NO_INTERPOLATION_PREFIX flat
#define NO_INTERPOLATION_SUFFIX

#define PARAMS_ARG_DECL
#define PARAMS_ARG


	#define inVs_vertexId gl_VertexIndex

#define inVs_vertex vertex
#define inVs_normal normal
#define inVs_tangent tangent
#define inVs_binormal binormal
#define inVs_blendWeights blendWeights
#define inVs_blendIndices blendIndices
#define inVs_qtangent qtangent
#define inVs_colour colour


	#define inVs_drawId drawId


#define finalDrawId inVs_drawId


	#define inVs_uv0 uv0

#define outVs_Position gl_Position
#define outVs_viewportIndex gl_ViewportIndex
#define outVs_clipDistance0 gl_ClipDistance[0]

#define gl_SampleMaskIn0 gl_SampleMaskIn[0]
#define reversebits bitfieldReverse

#define outPs_colour0 outColour


	#define OGRE_SampleArray2D( tex, sampler, uv, arrayIdx ) texture( sampler2DArray( tex, sampler ), vec3( uv, arrayIdx ) )
	#define OGRE_SampleArray2DLevel( tex, sampler, uv, arrayIdx, lod ) textureLod( sampler2DArray( tex, sampler ), vec3( uv, arrayIdx ), lod )
	#define OGRE_SampleArrayCubeLevel( tex, sampler, uv, arrayIdx, lod ) textureLod( samplerCubeArray( tex, sampler ), vec4( uv, arrayIdx ), lod )
	#define OGRE_SampleArray2DGrad( tex, sampler, uv, arrayIdx, ddx, ddy ) textureGrad( sampler2DArray( tex, sampler ), vec3( uv, arrayIdx ), ddx, ddy )

	#define OGRE_Load2DF16( tex, iuv, lod ) midf4_c( texelFetch( tex, ivec2( iuv ), lod ) )
	#define OGRE_Load2DMSF16( tex, iuv, subsample ) midf4_c( texelFetch( tex, iuv, subsample ) )
	#define OGRE_SampleArray2DF16( tex, sampler, uv, arrayIdx ) midf4_c( texture( sampler2DArray( tex, sampler ), vec3( uv, arrayIdx ) ) )
	#define OGRE_SampleArray2DLevelF16( tex, sampler, uv, arrayIdx, lod ) midf4_c( textureLod( sampler2DArray( tex, sampler ), vec3( uv, arrayIdx ), lod ) )
	#define OGRE_SampleArrayCubeLevelF16( tex, sampler, uv, arrayIdx, lod ) midf4_c( textureLod( samplerCubeArray( tex, sampler ), vec4( uv, arrayIdx ), lod ) )
	#define OGRE_SampleArray2DGradF16( tex, sampler, uv, arrayIdx, ddx, ddy ) midf4_c( textureGrad( sampler2DArray( tex, sampler ), vec3( uv, arrayIdx ), ddx, ddy ) )

	float4 OGRE_Sample( texture2D t, sampler s, float2 uv ) { return texture( sampler2D( t, s ), uv ); }
	float4 OGRE_Sample( texture3D t, sampler s, float3 uv ) { return texture( sampler3D( t, s ), uv ); }
	float4 OGRE_Sample( textureCube t, sampler s, float3 uv ) { return texture( samplerCube( t, s ), uv ); }

	float4 OGRE_SampleLevel( texture2D t, sampler s, float2 uv, float lod ) { return textureLod( sampler2D( t, s ), uv, lod ); }
	float4 OGRE_SampleLevel( texture3D t, sampler s, float3 uv, float lod ) { return textureLod( sampler3D( t, s ), uv, lod ); }
	float4 OGRE_SampleLevel( textureCube t, sampler s, float3 uv, float lod ) { return textureLod( samplerCube( t, s ), uv, lod ); }

	float4 OGRE_SampleGrad( texture2D t, sampler s, float2 uv, float2 myDdx, float2 myDdy ) { return textureGrad( sampler2D( t, s ), uv, myDdx, myDdy ); }
	float4 OGRE_SampleGrad( texture3D t, sampler s, float3 uv, float3 myDdx, float3 myDdy ) { return textureGrad( sampler3D( t, s ), uv, myDdx, myDdy ); }
	float4 OGRE_SampleGrad( textureCube t, sampler s, float3 uv, float3 myDdx, float3 myDdy ) { return textureGrad( samplerCube( t, s ), uv, myDdx, myDdy ); }

	midf4 OGRE_SampleF16( texture2D t, sampler s, float2 uv ) { return midf4_c( texture( sampler2D( t, s ), uv ) ); }
	midf4 OGRE_SampleF16( texture3D t, sampler s, float3 uv ) { return midf4_c( texture( sampler3D( t, s ), uv ) ); }
	midf4 OGRE_SampleF16( textureCube t, sampler s, float3 uv ) { return midf4_c( texture( samplerCube( t, s ), uv ) ); }

	midf4 OGRE_SampleLevelF16( texture2D t, sampler s, float2 uv, float lod ) { return midf4_c( textureLod( sampler2D( t, s ), uv, lod ) ); }
	midf4 OGRE_SampleLevelF16( texture3D t, sampler s, float3 uv, float lod ) { return midf4_c( textureLod( sampler3D( t, s ), uv, lod ) ); }
	midf4 OGRE_SampleLevelF16( textureCube t, sampler s, float3 uv, float lod ) { return midf4_c( textureLod( samplerCube( t, s ), uv, lod ) ); }

	midf4 OGRE_SampleGradF16( texture2D t, sampler s, float2 uv, float2 myDdx, float2 myDdy ) { return midf4_c( textureGrad( sampler2D( t, s ), uv, myDdx, myDdy ) ); }
	midf4 OGRE_SampleGradF16( texture3D t, sampler s, float3 uv, float3 myDdx, float3 myDdy ) { return midf4_c( textureGrad( sampler3D( t, s ), uv, myDdx, myDdy ) ); }
	midf4 OGRE_SampleGradF16( textureCube t, sampler s, float3 uv, float3 myDdx, float3 myDdy ) { return midf4_c( textureGrad( samplerCube( t, s ), uv, myDdx, myDdy ) ); }

#define OGRE_ddx( val ) dFdx( val )
#define OGRE_ddy( val ) dFdy( val )
#define OGRE_Load2D( tex, iuv, lod ) texelFetch( tex, ivec2( iuv ), lod )
#define OGRE_LoadArray2D( tex, iuv, arrayIdx, lod ) texelFetch( tex, ivec3( iuv, arrayIdx ), lod )
#define OGRE_Load2DMS( tex, iuv, subsample ) texelFetch( tex, iuv, subsample )

#define OGRE_Load3D( tex, iuv, lod ) texelFetch( tex, ivec3( iuv ), lod )


	#define bufferFetch1( buffer, idx ) texelFetch( buffer, idx ).x



	#define OGRE_SAMPLER_ARG_DECL( samplerName ) , sampler samplerName
	#define OGRE_SAMPLER_ARG( samplerName ) , samplerName

	#define CONST_BUFFER( bufferName, bindingPoint ) layout_constbuffer(ogre_B##bindingPoint) uniform bufferName
	#define CONST_BUFFER_STRUCT_BEGIN( structName, bindingPoint ) layout_constbuffer(ogre_B##bindingPoint) uniform structName
	#define CONST_BUFFER_STRUCT_END( variableName ) variableName

	#define ReadOnlyBufferF( slot, varType, varName ) layout(std430, ogre_R##slot) readonly restrict buffer _##varName { varType varName[]; }
	#define ReadOnlyBufferU ReadOnlyBufferF
	#define readOnlyFetch( bufferVar, idx ) bufferVar[idx]
	#define readOnlyFetch1( bufferVar, idx ) bufferVar[idx]



#define OGRE_Texture3D_float4 texture3D

#define OGRE_ArrayTex( declType, varName, arrayCount ) declType varName[arrayCount]

#define FLAT_INTERPOLANT( decl, bindingPoint ) flat decl
#define INTERPOLANT( decl, bindingPoint ) decl

#define OGRE_OUT_REF( declType, variableName ) out declType variableName
#define OGRE_INOUT_REF( declType, variableName ) inout declType variableName

#define OGRE_ARRAY_START( type ) type[](
#define OGRE_ARRAY_END )



out gl_PerVertex
{
	vec4 gl_Position;

};

layout(std140) uniform;


	
		#define worldViewMat passBuf.view
	

	
#define UNPACK_MAT4( matrixBuf, pixelIdx ) mat4( matrixBuf[(pixelIdx) << 2u], matrixBuf[((pixelIdx) << 2u)+1u], matrixBuf[((pixelIdx) << 2u)+2u], matrixBuf[((pixelIdx) << 2u)+3u] )

	
#define UNPACK_MAT4x3( matrixBuf, pixelIdx ) mat3x4( matrixBuf[(pixelIdx) << 2u], matrixBuf[((pixelIdx) << 2u)+1u], matrixBuf[((pixelIdx) << 2u)+2u] )


	// START UNIFORM DECLARATION
	
struct ShadowReceiverData
{
	float4x4 texViewProj;

	float2 shadowDepthRange;
	float normalOffsetBias;
	float padding;
	float4 invShadowMapSize;
};

struct Light
{
	
		float4 position;	//.w contains the objLightMask
	
	float4 diffuse;		//.w contains numNonCasterDirectionalLights
	float3 specular;


#define lightTexProfileIdx spotDirection.w
};

#define numNonCasterDirectionalLights lights[0].diffuse.w

#define areaLightDiffuseMipmapStart areaApproxLights[0].diffuse.w
#define areaLightNumMipmapsSpecFactor areaApproxLights[0].specular.w

#define numAreaApproxLights areaApproxLights[0].doubleSided.y
#define numAreaApproxLightsWithMask areaApproxLights[0].doubleSided.z

#define numAreaLtcLights areaLtcLights[0].points[0].w
#define numAreaLtcLights areaLtcLights[0].points[0].w

struct AreaLight
{
	
		float4 position;	//.w contains the objLightMask
	
	float4 diffuse;		//[0].w contains diffuse mipmap start
	float4 specular;	//[0].w contains mipmap scale
	float4 attenuation;	//.w contains texture array idx
	//Custom 2D Shape:
	//  direction.xyz direction
	//  direction.w invHalfRectSize.x
	//  tangent.xyz tangent
	//  tangent.w invHalfRectSize.y
	float4 direction;
	float4 tangent;
	float4 doubleSided;	//.y contains numAreaApproxLights
						//.z contains numAreaApproxLightsWithMask
	
};

struct AreaLtcLight
{
	
		float4 position;	//.w contains the objLightMask
	
	float4 diffuse;			//.w contains attenuation range
	float4 specular;		//.w contains doubleSided
	float4 points[4];		//.w contains numAreaLtcLights
							//points[1].w, points[2].w, points[3].w contain obbFadeFactorLtc.xyz
	
};





//Uniforms that change per pass
CONST_BUFFER_STRUCT_BEGIN( PassBuffer, 0 )
{
	//Vertex shader (common to both receiver and casters)

	float4x4 viewProj;







	//Vertex shader
	float4x4 view;
	

	

	//-------------------------------------------------------------------------

	//Pixel shader
	float3x3 invViewMatCubemap;


	float4 pccVctMinDistance_invPccVctInvDistance_rightEyePixelStartX_envMapNumMipmaps;

	float4 aspectRatio_planarReflNumMips_unused2;

	float2 invWindowRes;
	float2 windowResolution;














	Light lights[1];
	AreaLight areaApproxLights[4];
	
// !use_light_buffers





	



	
	

	

#define pccVctMinDistance		pccVctMinDistance_invPccVctInvDistance_rightEyePixelStartX_envMapNumMipmaps.x
#define invPccVctInvDistance	pccVctMinDistance_invPccVctInvDistance_rightEyePixelStartX_envMapNumMipmaps.y
#define rightEyePixelStartX		pccVctMinDistance_invPccVctInvDistance_rightEyePixelStartX_envMapNumMipmaps.z
#define envMapNumMipmaps		pccVctMinDistance_invPccVctInvDistance_rightEyePixelStartX_envMapNumMipmaps.w

#define aspectRatio			aspectRatio_planarReflNumMips_unused2.x
#define planarReflNumMips	aspectRatio_planarReflNumMips_unused2.y
}
CONST_BUFFER_STRUCT_END( passBuf );



#define light0Buf		passBuf
#define light1Buf		passBuf
#define light2Buf		passBuf

// use_light_buffers


	
		//Uniforms that change per Item/Entity
		CONST_BUFFER( InstanceBuffer, 2 )
		{
			//.x =
			//The lower 9 bits contain the material's start index.
			//The higher 23 bits contain the world matrix start index.
			//
			//.y =
			//shadowConstantBias. Send the bias directly to avoid an
			//unnecessary indirection during the shadow mapping pass.
			//Must be loaded with uintBitsToFloat
			//
			//.z =
			//lightMask. Ogre must have been compiled with OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
			
				uint4 worldMaterialIdx[4096];
			
		};
	
	
	
	// END UNIFORM DECLARATION

	

    
			
			

			
		
			// Perform normal offset bias. See https://github.com/OGRECave/ogre-next/issues/100
			INLINE float3 getNormalOffsetBias( midf3 worldNormal, midf3 viewSpaceNormal,
											   float3 lightDir, float shadowMapTexSize,
											   float depthRange, float normalOffsetBias
			
											   )
			
			{
				float tmpNdotL = saturate( dot( lightDir.xyz, float3( viewSpaceNormal.xyz ) ) );

				

				return ( ( 1.0f - tmpNdotL ) * normalOffsetBias * float3( worldNormal.xyz ) * shadowMapTexSize );
			}
		
			// Perform normal offset bias. See https://github.com/OGRECave/ogre-next/issues/100
			INLINE float3 getNormalOffsetBias( midf3 worldNormal, midf3 viewSpaceNormal,
											   float3 lightDir, float shadowMapTexSize,
											   float depthRange, float normalOffsetBias
			
											   , float2 minUV, float2 maxUV )
			
			{
				float tmpNdotL = saturate( dot( lightDir.xyz, float3( viewSpaceNormal.xyz ) ) );

				
					shadowMapTexSize /= maxUV.x - minUV.x;
				

				return ( ( 1.0f - tmpNdotL ) * normalOffsetBias * float3( worldNormal.xyz ) * shadowMapTexSize );
			}
		
	
		
	



vulkan_layout( OGRE_POSITION ) in vec4 vertex;

vulkan_layout( OGRE_NORMAL ) in float3 normal;





	vulkan_layout( OGRE_BLENDINDICES )in uvec4 blendIndices;
	vulkan_layout( OGRE_BLENDWEIGHT )in vec4 blendWeights;



	vulkan_layout( OGRE_TEXCOORD0 ) in vec2 uv0;


	vulkan_layout( OGRE_DRAWID ) in uint drawId;





	vulkan_layout( location = 0 ) out block
	{
		
	
		
			FLAT_INTERPOLANT( ushort drawId, 0 );
		
	

	
		
			INTERPOLANT( float3 pos, 1 );
			INTERPOLANT( midf3 normal, 2 );
			
		
		
			INTERPOLANT( float2 uv0, 3 );

		
			
		

		
		

		

		
	
	

	} outVs;


// START UNIFORM GL DECLARATION
ReadOnlyBufferF( 0, float4, worldMatBuf );



// END UNIFORM GL DECLARATION

void main()
{

    
	
	// Define inputPos using inVs_vertex.
	
		#define inputPos inVs_vertex
	

	// Define inputNormal and inputTangent using inVs_normal, inVs_tangent, inVs_qtangent
	
		
			midf3 inputNormal = midf3_c( inVs_normal ); // We need inputNormal as lvalue for PoseTransform
		
		
	

	

	

	

	
	uint _idx = (inVs_blendIndices[0] << 1u) + inVs_blendIndices[0]; //inVs_blendIndices[0] * 3u; a 32-bit int multiply is 4 cycles on GCN! (and mul24 is not exposed to GLSL...)
	uint matStart = worldMaterialIdx[inVs_drawId].x >> 9u;
	float4 worldMat[3];
	worldMat[0] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 0u) );
	worldMat[1] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 1u) );
	worldMat[2] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 2u) );
	float4 worldPos;
	worldPos.x = dot( worldMat[0], inputPos );
	worldPos.y = dot( worldMat[1], inputPos );
	worldPos.z = dot( worldMat[2], inputPos );
	worldPos.xyz *= inVs_blendWeights[0];
    
		midf3 worldNorm;
		worldNorm.x = dot( midf3_c( worldMat[0].xyz ), inputNormal );
		worldNorm.y = dot( midf3_c( worldMat[1].xyz ), inputNormal );
		worldNorm.z = dot( midf3_c( worldMat[2].xyz ), inputNormal );
		worldNorm *= midf_c( inVs_blendWeights[0] );
	
	

	
	
		float4 tmp4;
		tmp4.w = 1.0;
		midf3 tmp3;
	//!NeedsMoreThan1BonePerVertex
	
		_idx = (inVs_blendIndices[1] << 1u) + inVs_blendIndices[1]; //inVs_blendIndices[1] * 3; a 32-bit int multiply is 4 cycles on GCN! (and mul24 is not exposed to GLSL...)
		worldMat[0] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 0u) );
		worldMat[1] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 1u) );
		worldMat[2] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 2u) );
		tmp4.x = dot( worldMat[0], inputPos );
		tmp4.y = dot( worldMat[1], inputPos );
		tmp4.z = dot( worldMat[2], inputPos );
		worldPos.xyz += (tmp4 * inVs_blendWeights[1]).xyz;
		
			tmp3.x = dot( midf3_c( worldMat[0].xyz ), inputNormal );
			tmp3.y = dot( midf3_c( worldMat[1].xyz ), inputNormal );
			tmp3.z = dot( midf3_c( worldMat[2].xyz ), inputNormal );
			worldNorm += tmp3.xyz * midf_c( inVs_blendWeights[1] );
		
		
	
		_idx = (inVs_blendIndices[2] << 1u) + inVs_blendIndices[2]; //inVs_blendIndices[2] * 3; a 32-bit int multiply is 4 cycles on GCN! (and mul24 is not exposed to GLSL...)
		worldMat[0] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 0u) );
		worldMat[1] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 1u) );
		worldMat[2] = readOnlyFetch( worldMatBuf, int(matStart + _idx + 2u) );
		tmp4.x = dot( worldMat[0], inputPos );
		tmp4.y = dot( worldMat[1], inputPos );
		tmp4.z = dot( worldMat[2], inputPos );
		worldPos.xyz += (tmp4 * inVs_blendWeights[2]).xyz;
		
			tmp3.x = dot( midf3_c( worldMat[0].xyz ), inputNormal );
			tmp3.y = dot( midf3_c( worldMat[1].xyz ), inputNormal );
			tmp3.z = dot( midf3_c( worldMat[2].xyz ), inputNormal );
			worldNorm += tmp3.xyz * midf_c( inVs_blendWeights[2] );
		
		
	

	worldPos.w = 1.0;

	
	
	//Lighting is in view space
		outVs.pos		= mul( worldPos, worldViewMat ).xyz;
		outVs.normal	= mul( worldNorm, toMidf3x3( worldViewMat ) );
	
	
        
			
				outVs_Position = mul( worldPos, passBuf.viewProj );
			
		
	


	
				float3 normalOffsetBias = float3( 0, 0, 0 );
				

				
			
	

	

	/// hlms_uv_count will be 0 on shadow caster passes w/out alpha test
	
		outVs.uv0 = inVs_uv0;


	
		outVs.drawId = inVs_drawId;
	


	

	

	

	
}
