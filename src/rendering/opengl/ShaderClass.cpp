#include"ShaderClass.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

std::filesystem::path getExecutableDir()
{
	namespace fs = std::filesystem;
#ifdef _WIN32
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	return fs::path(path).parent_path();
#else
	char path[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
	if (len != -1)
	{
		path[len] = '\0';
		return fs::path(path).parent_path();
	}
	return fs::current_path();
#endif
}

std::string get_file_contents(const char* filename)
{
	namespace fs = std::filesystem;

	auto read_stream = [](std::ifstream& in) {
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(static_cast<size_t>(in.tellg()));
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return contents;
	};

	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		return read_stream(in);
	}

	fs::path exeDir = getExecutableDir();
	fs::path shadersPath = exeDir / "shaders" / filename;
	in.open(shadersPath, std::ios::binary);
	if (in)
	{
		return read_stream(in);
	}

#ifdef SHADER_DIR
	fs::path fallback = fs::path(SHADER_DIR) / filename;
	in.open(fallback, std::ios::binary);
	if (in)
	{
		return read_stream(in);
	}
#endif

	throw std::runtime_error("Failed to open shader file: " + fs::path(filename).string());
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	compileErrors(vertexShader, "VERTEX");

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	compileErrors(fragmentShader, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

void Shader::Activate()
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}

void Shader::compileErrors(unsigned int shader, const char* type)
{
	GLint hasCompiled;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}
