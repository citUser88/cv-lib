#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "vulkan/vulkan.hpp"
#include "vulkan/SPIRV/GlslangtoSpv.h"
#include "logging.h"

static const std::string vertexShaderText = R"(
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform bufferVals
{
    mat4 mvp;
} myBufferVals;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;
layout (location = 0) out vec4 outColor;

void main()
{
    outColor = inColor;
    gl_Position = myBufferVals.mvp * pos;
}
)";

static const std::string fragmentShaderText = R"(
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 color;
layout (location = 0) out vec4 outColor;

void main()
{
   outColor = color;
}
)";

vk::UniqueDevice uqDev4SPIRV;

class O2SPIRV
{
public:
	O2SPIRV()
	{
		try
		{	
			memset(&resource, 0x0, sizeof(resource));
			glslang::InitializeProcess();

			std::vector<unsigned int> vertexShaderSPV;
			bool ok = GLSLtoSPV(vk::ShaderStageFlagBits::eVertex, vertexShaderText, vertexShaderSPV);
			assert(ok);

			vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(),
				vertexShaderSPV.size() * sizeof(unsigned int), vertexShaderSPV.data());

			vk::UniqueShaderModule vertexShaderModule = uqDev4SPIRV->createShaderModuleUnique(vertexShaderModuleCreateInfo);

			std::vector<unsigned int> fragmentShaderSPV;
			ok = GLSLtoSPV(vk::ShaderStageFlagBits::eFragment, fragmentShaderText, fragmentShaderSPV);
			assert(ok);

			vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(),
				fragmentShaderSPV.size() * sizeof(unsigned int), fragmentShaderSPV.data());

			glslang::FinalizeProcess();
		}
		catch (vk::SystemError err)
		{
			logString_to_devLog("O2SPIRV: Encountered a system error... :l", true);
			std::string errString = err.what();
			std::string logerrString("O2SPIRV: this error: " + errString + "...\n");			
			logString_to_devLog(logerrString, true);
		}
		catch (std::runtime_error err)
		{
			logString_to_devLog("O2SV: No Bueno, something caused a  -RUNTIME- ERROR to occur. -> BYEBYE", true);
			exit(-1);
		}
		catch (...)
		{
			logString_to_devLog("Really bad, UNKNOWN error. -> TOODLES", true);
			exit(-1);
		}		
	}	
	bool GLSLtoSPV(const vk::ShaderStageFlagBits shaderType, std::string const& glslShader, std::vector<unsigned int> &spvShader)
	{
		EShLanguage stage = translateShaderStage(shaderType);

		const char *shaderStrings[1];
		shaderStrings[0] = glslShader.data();

		glslang::TShader shader(stage);
		shader.setStrings(shaderStrings, 1);
		
		init_resource();

		// Rules for parsing GLSL
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

		if (!shader.parse(&resource, 100, false, messages))
		{
			puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			return false;
		}

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages))
		{
			puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			fflush(stdout);
			return false;
		}

		glslang::GlslangToSpv(*program.getIntermediate(stage), spvShader);
		return true;
	}
private:	
	TBuiltInResource resource;

	EShLanguage translateShaderStage(vk::ShaderStageFlagBits stage)
	{
		switch (stage)
		{
			case vk::ShaderStageFlagBits::eVertex:                  return EShLangVertex;
			case vk::ShaderStageFlagBits::eTessellationControl:     return EShLangTessControl;
			case vk::ShaderStageFlagBits::eTessellationEvaluation:  return EShLangTessEvaluation;
			case vk::ShaderStageFlagBits::eGeometry:                return EShLangGeometry;
			case vk::ShaderStageFlagBits::eFragment:                return EShLangFragment;
			case vk::ShaderStageFlagBits::eCompute:                 return EShLangCompute;
			default:
				assert(false && "Unknown shader stage");
			return EShLangVertex;
		}
	}
	void init_resource()
	{
		resource.maxLights = 32;
		resource.maxClipPlanes = 6;
		resource.maxTextureUnits = 32;
		resource.maxTextureCoords = 32;
		resource.maxVertexAttribs = 64;
		resource.maxVertexUniformComponents = 4096;
		resource.maxVaryingFloats = 64;
		resource.maxVertexTextureImageUnits = 32;
		resource.maxCombinedTextureImageUnits = 80;
		resource.maxTextureImageUnits = 32;
		resource.maxFragmentUniformComponents = 4096;
		resource.maxDrawBuffers = 32;
		resource.maxVertexUniformVectors = 128;
		resource.maxVaryingVectors = 8;
		resource.maxFragmentUniformVectors = 16;
		resource.maxVertexOutputVectors = 16;
		resource.maxFragmentInputVectors = 15;
		resource.minProgramTexelOffset = -8;
		resource.maxProgramTexelOffset = 7;
		resource.maxClipDistances = 8;
		resource.maxComputeWorkGroupCountX = 65535;
		resource.maxComputeWorkGroupCountY = 65535;
		resource.maxComputeWorkGroupCountZ = 65535;
		resource.maxComputeWorkGroupSizeX = 1024;
		resource.maxComputeWorkGroupSizeY = 1024;
		resource.maxComputeWorkGroupSizeZ = 64;
		resource.maxComputeUniformComponents = 1024;
		resource.maxComputeTextureImageUnits = 16;
		resource.maxComputeImageUniforms = 8;
		resource.maxComputeAtomicCounters = 8;
		resource.maxComputeAtomicCounterBuffers = 1;
		resource.maxVaryingComponents = 60;
		resource.maxVertexOutputComponents = 64;
		resource.maxGeometryInputComponents = 64;
		resource.maxGeometryOutputComponents = 128;
		resource.maxFragmentInputComponents = 128;
		resource.maxImageUnits = 8;
		resource.maxCombinedImageUnitsAndFragmentOutputs = 8;
		resource.maxCombinedShaderOutputResources = 8;
		resource.maxImageSamples = 0;
		resource.maxVertexImageUniforms = 0;
		resource.maxTessControlImageUniforms = 0;
		resource.maxTessEvaluationImageUniforms = 0;
		resource.maxGeometryImageUniforms = 0;
		resource.maxFragmentImageUniforms = 8;
		resource.maxCombinedImageUniforms = 8;
		resource.maxGeometryTextureImageUnits = 16;
		resource.maxGeometryOutputVertices = 256;
		resource.maxGeometryTotalOutputComponents = 1024;
		resource.maxGeometryUniformComponents = 1024;
		resource.maxGeometryVaryingComponents = 64;
		resource.maxTessControlInputComponents = 128;
		resource.maxTessControlOutputComponents = 128;
		resource.maxTessControlTextureImageUnits = 16;
		resource.maxTessControlUniformComponents = 1024;
		resource.maxTessControlTotalOutputComponents = 4096;
		resource.maxTessEvaluationInputComponents = 128;
		resource.maxTessEvaluationOutputComponents = 128;
		resource.maxTessEvaluationTextureImageUnits = 16;
		resource.maxTessEvaluationUniformComponents = 1024;
		resource.maxTessPatchComponents = 120;
		resource.maxPatchVertices = 32;
		resource.maxTessGenLevel = 64;
		resource.maxViewports = 16;
		resource.maxVertexAtomicCounters = 0;
		resource.maxTessControlAtomicCounters = 0;
		resource.maxTessEvaluationAtomicCounters = 0;
		resource.maxGeometryAtomicCounters = 0;
		resource.maxFragmentAtomicCounters = 8;
		resource.maxCombinedAtomicCounters = 8;
		resource.maxAtomicCounterBindings = 1;
		resource.maxVertexAtomicCounterBuffers = 0;
		resource.maxTessControlAtomicCounterBuffers = 0;
		resource.maxTessEvaluationAtomicCounterBuffers = 0;
		resource.maxGeometryAtomicCounterBuffers = 0;
		resource.maxFragmentAtomicCounterBuffers = 1;
		resource.maxCombinedAtomicCounterBuffers = 1;
		resource.maxAtomicCounterBufferSize = 16384;
		resource.maxTransformFeedbackBuffers = 4;
		resource.maxTransformFeedbackInterleavedComponents = 64;
		resource.maxCullDistances = 8;
		resource.maxCombinedClipAndCullDistances = 8;
		resource.maxSamples = 4;
		resource.limits.nonInductiveForLoops = 1;
		resource.limits.whileLoops = 1;
		resource.limits.doWhileLoops = 1;
		resource.limits.generalUniformIndexing = 1;
		resource.limits.generalAttributeMatrixVectorIndexing = 1;
		resource.limits.generalVaryingIndexing = 1;
		resource.limits.generalSamplerIndexing = 1;
		resource.limits.generalVariableIndexing = 1;
		resource.limits.generalConstantMatrixVectorIndexing = 1;
}
};