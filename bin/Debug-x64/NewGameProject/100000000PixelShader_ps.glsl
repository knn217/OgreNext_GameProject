#if 0
	***	hlms_uv_count0	2
	***	hlms_pose	0
	***	uv_emissive	0
	***	first_valid_detail_map_nm	4
	***	fresnel_scalar	0
	***	envprobe_map_sampler	3
	***	hlms_uv_count	1
	***	uv_detail_nm1	0
	***	uv_detail_nm2	0
	***	areaLightMasks	1
	***	envMapRegSampler	3
	***	emissive_map_sampler	3
	***	precision_mode	-2126167738
	***	BRDF_Default	1
	***	set0_texture_slot_end	3
	***	uv_specular	0
	***	hlms_disable_stage	0
	***	NumPoseWeightVectors	0
	***	hlms_lights_area_approx	4
	***	specular_map_sampler	3
	***	hlms_forwardplus_flipY	1
	***	texcoord	4
	***	glsl	635204550
	***	NumPoseWeightVectorsB	0
	***	hlms_lights_directional	0
	***	samplerStateStart	3
	***	alpha_test	0
	***	roughness_map_sampler	3
	***	GGX_height_correlated	1
	***	glsles	1070293233
	***	metallic_workflow	0
	***	hw_gamma_write	1
	***	uv_detail_weight	0
	***	hlms_lights_area_tex_colour	1
	***	detail_map_nm2_sampler	3
	***	hlms_alpha_to_coverage	0
	***	cubemaps_as_diffuse_gi	1
	***	uv_detail_nm3	0
	***	NumPoseWeightVectorsC	0
	***	hlms_pose_normals	0
	***	hlms_pose_half	0
	***	hlms_lights_spot	1
	***	GL_ARB_shading_language_420pack	1
	***	detail_map1_sampler	3
	***	uv_normal	0
	***	relaxed	1726237731
	***	detail_map_nm3_sampler	3
	***	hlms_alphablend	0
	***	MoreThanOnePose	-1
	***	hlslvk	1841745752
	***	uv_detail3	0
	***	detail_map3_sampler	3
	***	normal_map	0
	***	NumPoseWeightVectorsA	0
	***	hlms_bones_per_vertex	3
	***	uv_detail1	0
	***	fresnel_workflow	0
	***	full32	-2126167738
	***	perceptual_roughness	1
	***	alpha_test_shadow_caster_only	0
	***	materials_per_buffer	240
	***	hw_gamma_read	1
	***	hlms_skeleton	1
	***	hlms_pssm_splits_minus_one	-1
	***	midf16	-1978079318
	***	syntax	-338983575
	***	NeedsMoreThan1BonePerVertex	2
	***	hlms_lights_point	1
	***	diffuse_map_sampler	3
	***	metal	-1698855755
	***	GL_ARB_base_instance	1
	***	detail_map_nm1_sampler	3
	***	s_lights_directional_non_caster	1
	***	uv_detail0	0
	***	uv_diffuse	0
	***	uv_detail_nm0	0
	***	detail_weight_map_sampler	3
	***	GL3+	450
	***	detail_map_nm0_sampler	3
	***	hlms_render_depth_only	0
	***	num_pass_const_buffers	3
	***	normal_map_tex_sampler	3
	***	uv_detail2	0
	***	MoreThanOnePoseWeightVector	-1
	***	detail_map2_sampler	3
	***	detail_map0_sampler	3
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




	#define UV_DIFFUSE(x) (x)
	#define UV_NORMAL(x) (x)
	#define UV_SPECULAR(x) (x)
	#define UV_ROUGHNESS(x) (x)
	#define UV_DETAIL_WEIGHT(x) (x)
	#define UV_DETAIL0(x) (x)
	#define UV_DETAIL1(x) (x)
	#define UV_DETAIL2(x) (x)
	#define UV_DETAIL3(x) (x)
	#define UV_DETAIL_NM0(x) (x)
	#define UV_DETAIL_NM1(x) (x)
	#define UV_DETAIL_NM2(x) (x)
	#define UV_DETAIL_NM3(x) (x)
	#define UV_EMISSIVE(x) (x)
	


layout(std140) uniform;


	
		
			layout(location = 0, index = 0) out midf4 outColour;
		
		
		
	








		
		
			layout( ogre_t1 ) uniform texture2DArray areaLightMasks;
			layout( ogre_s1 ) uniform sampler areaLightMasksSampler;
		
		
		
	





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


		
		
//Uniforms that change per Item/Entity, but change very infrequently
struct Material
{
	/* kD is already divided by PI to make it energy conserving.
	  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
	*/
	float4 bgDiffuse;
	float4 kD; //kD.w is alpha_test_threshold
	float4 kS; //kS.w is roughness
	//Fresnel coefficient, may be per colour component (float3) or scalar (float)
	//F0.w is transparency
	float4 F0;
	float4 normalWeights;
	float4 cDetailWeights;
	float4 detailOffsetScale[4];
	float4 emissive;		//emissive.w contains mNormalMapWeight.
	float refractionStrength;
	float clearCoat;
	float clearCoatRoughness;
	float _padding1;
	float4 userValue[3];

	
		uint4 indices0_3;
		uint4 indices4_7;
	

	
};
	

	
		CONST_BUFFER( MaterialBuf, 1 )
		{
			Material materialArray[240];
		};
	

		
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

	
		#define float_fresnel midf
		#define float_fresnel_c( x ) midf_c( x )
		#define make_float_fresnel( x ) midf_c( x )
	

	
	
		#define OGRE_DEPTH_CMP_GE( a, b ) (a) <= (b)
		#define OGRE_DEPTH_DEFAULT_CLEAR 0.0
	


	
		#define PASSBUF_ARG_DECL
		#define PASSBUF_ARG
	

	

	struct PixelData
	{
		
			midf3 normal;
			
				#define geomNormal normal
			
			
			midf4	diffuse;
			midf3	specular;

			

			midf	perceptualRoughness;
			midf	roughness;
			float_fresnel	F0;

			
				midf3	viewDir;
				midf	NdotV;
			

			
		

		
	};

	#define SampleDetailWeightMap( tex, sampler, uv, arrayIdx ) OGRE_SampleArray2DF16( tex, sampler, uv, arrayIdx )
	
	
	
	
	
	

	

	

	

	
		

		
//Default BRDF
INLINE midf3 BRDF( midf3 lightDir, midf3 lightDiffuse, midf3 lightSpecular, PixelData pixelData PASSBUF_ARG_DECL )
{
	midf3 halfWay = normalize( lightDir + pixelData.viewDir );
	midf NdotL = saturate( dot( pixelData.normal, lightDir ) );
	midf NdotH = saturate( dot( pixelData.normal, halfWay ) );
	midf VdotH = saturate( dot( pixelData.viewDir, halfWay ) );

	midf sqR = pixelData.roughness * pixelData.roughness;

	//Geometric/Visibility term (Smith GGX Height-Correlated)

	midf Lambda_GGXV = NdotL * sqrt( (-pixelData.NdotV * sqR + pixelData.NdotV) * pixelData.NdotV + sqR );
	midf Lambda_GGXL = pixelData.NdotV * sqrt( (-NdotL * sqR + NdotL) * NdotL + sqR );

	midf G = _h( 0.5 ) / (( Lambda_GGXV + Lambda_GGXL + _h( 1e-6f ) ) * _h( 3.141592654 ));



	//Roughness/Distribution/NDF term (GGX)
	//Formula:
	//	Where alpha = roughness
	//	R = alpha^2 / [ PI * [ ( NdotH^2 * (alpha^2 - 1) ) + 1 ]^2 ]
	const midf f = ( NdotH * sqR - NdotH ) * NdotH + _h( 1.0 );
	midf R = sqR / (f * f); // f is guaranteed to not be 0 because we clamped pixelData.roughness

	const midf RG = R * G;


	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
    float_fresnel fresnelS = pixelData.F0 + pow( _h( 1.0 ) - VdotH, _h( 5.0 ) ) * (_h( 1.0 ) - pixelData.F0);

	//We should divide Rs by PI, but it was done inside G for performance
	midf3 Rs = ( fresnelS * RG ) * pixelData.specular.xyz;

	//Diffuse BRDF (*Normalized* Disney, see course_notes_moving_frostbite_to_pbr.pdf
	//"Moving Frostbite to Physically Based Rendering" Sebastien Lagarde & Charles de Rousiers)
	midf energyBias	= pixelData.perceptualRoughness * _h( 0.5 );
	midf energyFactor	= lerp( _h( 1.0 ), _h( 1.0 / 1.51 ), pixelData.perceptualRoughness );
	midf fd90			= energyBias + _h( 2.0 ) * VdotH * VdotH * pixelData.perceptualRoughness;
	midf lightScatter	= _h( 1.0 ) + (fd90 - _h( 1.0 )) * pow( _h( 1.0 ) - NdotL, _h( 5.0 ) );
	midf viewScatter	= _h( 1.0 ) + (fd90 - _h( 1.0 )) * pow( _h( 1.0 ) - pixelData.NdotV, _h( 5.0 ) );

	
		midf fresnelD = _h( 1.0f );
	

	//We should divide Rd by PI, but it is already included in kD
	midf3 Rd = (lightScatter * viewScatter * energyFactor * fresnelD) * pixelData.diffuse.xyz;

	
		return NdotL * (Rs * lightSpecular + Rd * lightDiffuse);
	
}

		
		
INLINE midf3 BRDF_AreaLightApprox
(
	midf3 lightDir, midf3 lightDiffuse, midf3 lightSpecular, PixelData pixelData
)
{
	midf3 halfWay= normalize( lightDir + pixelData.viewDir );
	midf NdotL = saturate( dot( pixelData.normal, lightDir ) );
	midf VdotH = saturate( dot( pixelData.viewDir, halfWay ) );

	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
	float_fresnel fresnelS = pixelData.F0 + pow( _h( 1.0 ) - VdotH, _h( 5.0 ) ) * (_h( 1.0 ) - pixelData.F0);

	//We should divide Rs by PI, but it was done inside G for performance
	midf3 Rs = fresnelS * pixelData.specular.xyz * lightSpecular;

	//Diffuse BRDF (*Normalized* Disney, see course_notes_moving_frostbite_to_pbr.pdf
	//"Moving Frostbite to Physically Based Rendering" Sebastien Lagarde & Charles de Rousiers)
	midf energyBias	= pixelData.roughness * _h( 0.5 );
	midf energyFactor	= lerp( _h( 1.0 ), _h( 1.0 / 1.51 ), pixelData.roughness );
	midf fd90			= energyBias + _h( 2.0 ) * VdotH * VdotH * pixelData.roughness;
	midf lightScatter	= _h( 1.0 ) + (fd90 - _h( 1.0 )) * pow( _h( 1.0 ) - NdotL, _h( 5.0 ) );
	midf viewScatter	= _h( 1.0 ) + (fd90 - _h( 1.0 )) * pow( _h( 1.0 ) - pixelData.NdotV, _h( 5.0 ) );


	midf fresnelD = _h( 1.0f ) - fresnelS;


	//We should divide Rd by PI, but it is already included in kD
	midf3 Rd = (lightScatter * viewScatter * energyFactor * fresnelD) * pixelData.diffuse.xyz * lightDiffuse;

	return NdotL * (Rs + Rd);
}

	

	
	
	
	
	






vulkan_layout( location = 0 ) in block
{

	
		
			FLAT_INTERPOLANT( ushort drawId, 0 );
		
	

	
		
			INTERPOLANT( float3 pos, 1 );
			INTERPOLANT( midf3 normal, 2 );
			
		
		
			INTERPOLANT( float2 uv0, 3 );

		
			
		

		
		

		

		
	
	

} inPs;














	


















void main()
{
    
	
	
	

	
		PixelData pixelData;

		
	
		
            ushort materialId	= worldMaterialIdx[inPs.drawId].x & 0x1FFu;
            #define material materialArray[materialId]
		
	

		
	
		
		
		
		
		
		
	

		
	
		
		
		
		
		
		
		
		
		
	

		
		

		
	


		
	/// Sample detail maps and weight them against the weight map in the next foreach loop.
	


		
			
	/// DIFFUSE MAP
	
		/// If there are no diffuse maps, we must initialize it to some value.
		pixelData.diffuse.xyzw = midf4_c( material.bgDiffuse.xyzw );
	

	/// Blend the detail diffuse maps with the main diffuse.
	

	/// Apply the material's diffuse over the textures
	pixelData.diffuse.xyz *= midf3_c( material.kD.xyz );
	

	

		

		
	/// SPECUlAR MAP
	pixelData.specular.xyz = midf3_c( material.kS.xyz );
	
		pixelData.F0 = float_fresnel_c( material.F0.x );
		
		
	
	

		
		
	/// ROUGHNESS MAP
	pixelData.perceptualRoughness = midf_c( material.kS.w );
	



	
		pixelData.roughness = max( pixelData.perceptualRoughness * pixelData.perceptualRoughness, _h( 0.001f ) );
	




		

		
			
	
		// Geometric normal
		pixelData.normal = normalize( inPs.normal ) ;
	

			
	/// If there is no normal map, the first iteration must
	/// initialize pixelData.normal instead of try to merge with it.
	
		
		
	

	

	

	/// Blend the detail normal maps with the main normal.
	
	


			

			

			

			

		

		
			
	//Everything's in Camera space
	
		
			pixelData.viewDir	= midf3_c( normalize( -inPs.pos ) );
		
		pixelData.NdotV		= saturate( dot( pixelData.normal, pixelData.viewDir ) );
	

	
		midf3 finalColour = midf3_c(0, 0, 0);
	

	
		float3 lightDir;
		float fDistance;
		midf3 tmpColour;
		midf spotCosAngle;
	

	

	

	


			

			
				
	
	

	
		
			
				finalColour += BRDF( midf3_c( light0Buf.lights[0].position.xyz ),
									 midf3_c( light0Buf.lights[0].diffuse.xyz ),
									 midf3_c( light0Buf.lights[0].specular ), pixelData PASSBUF_ARG );
	

			

			

	


			

	



			
	
		#define AREA_LIGHTS_TEX_SWIZZLE xyz
	

	float3 projectedPosInPlane;

	for( int i=0; i<floatBitsToInt( light1Buf.numAreaApproxLights ); ++i )
	{
		lightDir = light1Buf.areaApproxLights[i].position.xyz - inPs.pos;
		projectedPosInPlane.xyz = inPs.pos - dot( -lightDir.xyz, light1Buf.areaApproxLights[i].direction.xyz ) *
											 light1Buf.areaApproxLights[i].direction.xyz;
		fDistance = length( lightDir );

		

		if( fDistance <= light1Buf.areaApproxLights[i].attenuation.x
			
		/*&& dot( -lightDir, light1Buf.areaApproxLights[i].direction.xyz ) > 0*/  )
		{
			projectedPosInPlane.xyz -= light1Buf.areaApproxLights[i].position.xyz;
			float3 areaLightBitangent = cross( light1Buf.areaApproxLights[i].tangent.xyz,
											   light1Buf.areaApproxLights[i].direction.xyz );
			float2 invHalfRectSize = float2( light1Buf.areaApproxLights[i].direction.w,
											 light1Buf.areaApproxLights[i].tangent.w );
			//lightUV is in light space, in range [-0.5; 0.5]
			float2 lightUVForTex;
			float2 lightUV;
			lightUV.x = dot( projectedPosInPlane.xyz, light1Buf.areaApproxLights[i].tangent.xyz );
			lightUV.y = dot( projectedPosInPlane.xyz, areaLightBitangent );
			lightUV.xy *= invHalfRectSize.xy /*/ sqrt( fDistance )*/;
			//Displace the UV by the normal to account for edge cases when
			//a surface is close and perpendicular to the light. This is fully a hack and
			//the values (e.g. 0.25) is completely eye balled.
			lightUVForTex.xy = lightUV.xy;
			lightUV.xy += float2( dot( ( light1Buf.areaApproxLights[i].tangent.xyz ), float3( pixelData.normal ) ),
								  dot( areaLightBitangent, float3( pixelData.normal ) ) ) * 3.75 * invHalfRectSize.xy;
			lightUV.xy = clamp( lightUV.xy, -0.5f, 0.5f );
			lightUVForTex = clamp( lightUVForTex.xy, -0.5f, 0.5f );
	//		float booster = 1.0f - smoothstep( 0.2f, 1.9f, max( abs( lightUV.x ), abs( lightUV.y ) ) );
	//		booster = 1.0f + booster * 2.25f;
			midf booster = lerp( _h( 1.0f ), _h( 4.0f ), pixelData.roughness );

		
			midf3 diffuseMask = midf3_c( 1.0f, 1.0f, 1.0f );
		
		
			if( i < floatBitsToInt( light1Buf.numAreaApproxLightsWithMask ) )
			{
				// 1 / (1 - 0.02) = 1.020408163
				float diffuseMipsLeft = light1Buf.areaLightNumMipmapsSpecFactor * 0.5 -
										light1Buf.areaLightDiffuseMipmapStart * 1.020408163f;
				diffuseMask = OGRE_SampleArray2DLevelF16( areaLightMasks, areaLightMasksSampler,
														  lightUVForTex + 0.5f,
														  light1Buf.areaApproxLights[i].attenuation.w,
														  light1Buf.areaLightDiffuseMipmapStart +
														  (pixelData.roughness - 0.02f) * diffuseMipsLeft ).AREA_LIGHTS_TEX_SWIZZLE;
			}
		

			float3 closestPoint = light1Buf.areaApproxLights[i].position.xyz +
					light1Buf.areaApproxLights[i].tangent.xyz * lightUV.x / invHalfRectSize.x +
					areaLightBitangent.xyz * lightUV.y / invHalfRectSize.y;

			float3 lightDir2 = lightDir / fDistance;
			lightDir = closestPoint.xyz - inPs.pos;
			fDistance= length( lightDir );

			midf3 toShapeLight = reflect( -pixelData.viewDir, pixelData.normal );
			midf denom = dot( toShapeLight, midf3_c( -light1Buf.areaApproxLights[i].direction.xyz ) );
			
				midf3 specCol = midf3_c( 0, 0, 0 );
			
			if( denom > 1e-6f || light1Buf.areaApproxLights[i].doubleSided.x != 0.0f )
			{
				float3 p0l0 = light1Buf.areaApproxLights[i].position.xyz - inPs.pos;
				float t = dot( p0l0, -light1Buf.areaApproxLights[i].direction.xyz ) / float( denom );
				if( t >= 0 )
				{
					float3 posInShape = inPs.pos.xyz + float3( toShapeLight.xyz ) * t - light1Buf.areaApproxLights[i].position.xyz;
					float2 reflClipSpace;
					reflClipSpace.x = dot( light1Buf.areaApproxLights[i].tangent.xyz, posInShape );
					reflClipSpace.y = dot( areaLightBitangent, posInShape );

					float specVal;
					specVal = 0.5f / (length( max( abs( reflClipSpace * invHalfRectSize ) - 0.5f, 0.0f ) ) + 0.5f);
					specVal = min( specVal, 1.0f );
					float areaPower = ((pixelData.roughness * 10.0f + 1.0f) * 0.005f) /
									  (pixelData.roughness * pixelData.roughness * pixelData.roughness);
					areaPower = min( areaPower, 512.0f ); //Prevent INFs.
					specVal = pow( specVal, areaPower ) * min( areaPower * areaPower, 1.0f );

					
						specCol = midf3_c( specVal, specVal, specVal );
					

					
						if( i < floatBitsToInt( light1Buf.numAreaApproxLightsWithMask ) )
						{
							specCol *= OGRE_SampleArray2DLevelF16( areaLightMasks, areaLightMasksSampler,
																   reflClipSpace * invHalfRectSize + 0.5f,
																   light1Buf.areaApproxLights[i].attenuation.w,
																   (pixelData.roughness - _h( 0.02f )) *
																   light1Buf.areaLightNumMipmapsSpecFactor ).AREA_LIGHTS_TEX_SWIZZLE;
						}
					
				}
			}

			lightDir *= 1.0 / fDistance;
			//float fAreaW = dot( lightDir, -light1Buf.areaApproxLights[i].direction.xyz ) * 0.5f + 0.5f;
			//lightDir = (-light1Buf.areaApproxLights[i].direction.xyz + lightDir) * 0.50f;
			//lightDir = lerp( lightDir2, lightDir, fAreaW );
			midf globalDot = midf_c( saturate( dot( -lightDir, light1Buf.areaApproxLights[i].direction.xyz ) ) );
			globalDot = light1Buf.areaApproxLights[i].doubleSided.x != 0.0f ? _h( 1.0f ) : globalDot;
			tmpColour = BRDF_AreaLightApprox( midf3_c( lightDir ),
											  midf3_c( light1Buf.areaApproxLights[i].diffuse.xyz ) * diffuseMask,
											  midf3_c( light1Buf.areaApproxLights[i].specular.xyz ) * specCol,
											  pixelData ) * ( globalDot * globalDot ) * booster;
			midf atten = midf_c( 1.0 / (0.5 + (light1Buf.areaApproxLights[i].attenuation.y + light1Buf.areaApproxLights[i].attenuation.z * fDistance) * fDistance ) );

			

			finalColour += tmpColour * atten;
			//finalColour.xyz = float3( dot( lightDir, pixelData.normal ) );
			//finalColour.xyz = float3( lightUV.xy + 0.5f, 0.0f );
			//finalColour.xyz = float3( closestPoint.xy + 0.5f, 0.0f );
		}
	}


			

			

			

			
			

			
			

			
	


			

			

			

			
	

	

	


			

			

			
		///!hlms_prepass

	///!hlms_normal || hlms_qtangent

	
		
			
				
					outPs_colour0.xyz	= finalColour;
				

				
					outPs_colour0.w		= _h( 1.0 );
				

							
				
			
		
	

	
}
///hlms_shadowcaster
