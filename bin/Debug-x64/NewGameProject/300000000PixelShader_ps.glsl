#if 0
	***	hlms_uv_count0	2
	***	hlms_pose	0
	***	hlms_uv_count	1
	***	textureMapsArray0	2
	***	precision_mode	-2126167738
	***	diffuse_map0_idx	0
	***	hlms_disable_stage	0
	***	texcoord	2
	***	glsl	635204550
	***	uv_diffuse0	0
	***	out_uv_count	1
	***	samplerStateStart	2
	***	alpha_test	0
	***	glsles	1070293233
	***	out_uv0_out_uv	0
	***	diffuse_map0	1
	***	out_uv0_texture_matrix	0
	***	hlms_alpha_to_coverage	0
	***	hlms_pose_normals	0
	***	hlms_pose_half	0
	***	GL_ARB_shading_language_420pack	1
	***	relaxed	1726237731
	***	hlms_alphablend	0
	***	num_textures	1
	***	hlslvk	1841745752
	***	hlms_shadow_uses_depth_texture	0
	***	diffuse_map0_array	1
	***	hlms_tangent	0
	***	hlms_bones_per_vertex	0
	***	full32	-2126167738
	***	out_uv0_tex_unit	0
	***	alpha_test_shadow_caster_only	0
	***	materials_per_buffer	1024
	***	hlms_skeleton	0
	***	midf16	-1978079318
	***	out_uv0_source_uv	0
	***	syntax	-338983575
	***	hlms_tangent4	0
	***	metal	-1698855755
	***	GL_ARB_base_instance	1
	***	diffuse_map	1
	***	GL3+	450
	***	hlms_render_depth_only	0
	***	hlms_qtangent	0
	***	is_texture0_array	1
	***	out_uv_half_count0	2
	***	diffuse	1
	***	out_uv_half_count	1
	***	diffuse_map0_sampler	0
	***	hlms_normal	0
	***	num_samplers	1
	***	glslvk	-338983575
	***	hlsl	-334286542
	***	GL_ARB_texture_buffer_range	1
	DONE DUMPING PROPERTIES
	***	blend_mode_idx3	@insertpiece( NormalNonPremul)
	***	uv_diffuse_swizzle0	xy
	***	blend_mode_idx11	@insertpiece( NormalNonPremul)
	***	blend_mode_idx2	@insertpiece( NormalNonPremul)
	***	blend_mode_idx14	@insertpiece( NormalNonPremul)
	***	blend_mode_idx5	@insertpiece( NormalNonPremul)
	***	blend_mode_idx8	@insertpiece( NormalNonPremul)
	***	blend_mode_idx10	@insertpiece( NormalNonPremul)
	***	blend_mode_idx9	@insertpiece( NormalNonPremul)
	***	blend_mode_idx15	@insertpiece( NormalNonPremul)
	***	blend_mode_idx6	@insertpiece( NormalNonPremul)
	***	blend_mode_idx7	@insertpiece( NormalNonPremul)
	***	blend_mode_idx1	@insertpiece( NormalNonPremul)
	***	blend_mode_idx0	@insertpiece( NormalNonPremul)
	***	blend_mode_idx12	@insertpiece( NormalNonPremul)
	***	blend_mode_idx4	@insertpiece( NormalNonPremul)
	***	blend_mode_idx13	@insertpiece( NormalNonPremul)
	***	diffuse_map0_tex_swizzle	xyzw
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



layout(std140) uniform;
#define FRAG_COLOR		0




layout(location = FRAG_COLOR, index = 0) out midf4 outColour;


// START UNIFORM DECLARATION

// END UNIFORM DECLARATION

	vulkan_layout( location = 0 ) in block
	{
		
	
		
			FLAT_INTERPOLANT( uint drawId, 0 );
		
		
		
			INTERPOLANT( float2 uv0, 1 );
	
	

	} inPs;



	
		
			
				layout( ogre_t2 ) uniform texture2DArray textureMapsArray0;
			
		
	



	
		layout( ogre_s2 ) uniform sampler samplerState0;



	
		#define DiffuseSampler0 samplerState0
		
			#define SampleDiffuse0( tex, sampler, uv, d ) OGRE_SampleArray2DF16( tex, sampler, uv, d )
			#define DiffuseTexture0 textureMapsArray0
			
		

		
			#define DiffuseUV0 inPs.uv0.xy
		
	

	// START UNIFORM DECLARATION
	
	
		
	struct Material
	{
		float4 alpha_test_threshold;
		float4 diffuse;

		
			uint4 indices0_3;
			uint4 indices4_7;
		

		
	};

	
		CONST_BUFFER( MaterialBuf, 1 )
		{
			Material materialArray[1024];
		};
	

		
		//Uniforms that change per Item/Entity
		CONST_BUFFER( InstanceBuffer, 2 )
		{
			//.x =
			//Contains the material's start index.
			//
			//.y =
			//shadowConstantBias. Send the bias directly to avoid an
			//unnecessary indirection during the shadow mapping pass.
			//Must be loaded with uintBitsToFloat
			//
			//.z =
			//Contains 0 or 1 to index into passBuf.viewProj[]. Only used
			//if hlms_identity_viewproj_dynamic is set.
			
				uint4 worldMaterialIdx[4096];
			
		};
	
	
	
	// END UNIFORM DECLARATION




void main()
{
	
	
		
	midf4 diffuseCol = midf4_c( 1.0f, 1.0f, 1.0f, 1.0f );

	
		
	
		
			ushort materialId	= worldMaterialIdx[inPs.drawId].x & 0x1FFu;
			#define material materialArray[materialId]
		
	

	

	

	// Decode diffuse indices (for array textures)
	
		ushort diffuseIdx0 = material.indices0_3.x & 0x0000FFFFu;
		
		
		
		
		
		
		
	

	
		// Load base image
		diffuseCol = SampleDiffuse0( DiffuseTexture0, DiffuseSampler0,
									 DiffuseUV0 , diffuseIdx0 ).xyzw;
	

	// Load each additional layer and blend it
	

	// Apply material colour
	
		
			diffuseCol *= midf4_c( material.diffuse );
		
	

	

	

	
		outPs_colour0 = diffuseCol;
	

	
	
	
}
