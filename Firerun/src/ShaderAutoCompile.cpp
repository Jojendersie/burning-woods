#pragma comment(lib, "d3dx9.lib")
#include <D3DX9.h>
#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>

#define _CRT_SECURE_NO_WARNINGS

int main()
{
	std::cout << "###############################################" << std::endl;
	std::cout << "Running Shader AutoCompile...\n\n";

	char aShaderDir[] = "Shader ASM\\*";

	std::ofstream ShaderHeader("include\\Shader.h");
	ShaderHeader << "// Auto generated shader opcodes\n\n";

	WIN32_FIND_DATA	FData;
	HANDLE			hSearch = FindFirstFile(aShaderDir, &FData);
	do
	{
		if(FData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		// get ending
		std::string Filename(FData.cFileName);
		std::string Ending = Filename.substr(Filename.find_last_of('.')+1);
		std::string FileWithoutEnding = Filename.substr(0, Filename.find_last_of('.'));

		if(Ending == "psh" || Ending == "vsh")
		{
			bool PixelShader = Ending == "psh";
			if(PixelShader)
				std::cout << "Found Pixelshader \"" << FData.cFileName << "\" .. compiling .." << std::endl;
			else
				std::cout << "Found Vertexshader \"" << FData.cFileName << "\" .. compiling .." << std::endl;

			ShaderHeader << "// " << Filename << ":" << std::endl;

			LPD3DXBUFFER pBuffer = nullptr;
			LPD3DXBUFFER pErrors = nullptr;
			D3DXAssembleShaderFromFile((std::string("Shader ASM\\") + FData.cFileName).c_str(), 0, 0, D3DXSHADER_OPTIMIZATION_LEVEL1, &pBuffer, &pErrors);

			if(pErrors == nullptr && pBuffer)
			{
				std::cout << "Succeded!" << std::endl;

				
				ShaderHeader << "const DWORD g_a" << FileWithoutEnding  << (PixelShader ? "PS" : "VS") << "[] = { ";
				DWORD* pDWORDs = static_cast<DWORD*>(pBuffer->GetBufferPointer());
				unsigned int NumTocken = pBuffer->GetBufferSize() / sizeof(DWORD);
				for(unsigned int i=0; i<NumTocken-1; ++i)
					ShaderHeader << pDWORDs[i] << ", ";
				ShaderHeader << pDWORDs[NumTocken-1] << " };\n\n";

			}
			else
			{
				if(pErrors)
					std::cout << "Some errors occured:\n" << static_cast<char*>(pErrors->GetBufferPointer()) << std::endl << std::endl;
				else
					std::cout << "Some errors occured: - No description - \n\n";
			}

			std::cout << std::endl;
		}

	} while(FindNextFile(hSearch,&FData));

	ShaderHeader.close();

	std::cout << "Shader AutoCompile finished!" << std::endl;
	std::cout << "###############################################\n\n";
}