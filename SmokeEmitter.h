#include <windows.h>          // Header File For Windows
#include <stdio.h>            // Header File For Standard Input/Output ( ADD )
#include <gl\gl.h>            // Header File For The OpenGL32 Library
#include <gl\glu.h>           // Header File For The GLu32 Library

#include <fstream>
#include <vector>

class SmokeEmitter
{
	typedef struct                      // Create A Structure For Particle
	{
		bool    active;                 // Active (Yes/No)
		float   life;                   // Particle Life
		float   fade;                   // Fade Speed
		float   x;                  // X Position
		float   y;                  // Y Position
		float   z;                  // Z Position
		float   xi;                 // X Direction
		float   yi;                 // Y Direction
		float   zi;                 // Z Direction
		float   r;                  // Red Value
		float   g;                  // Green Value
		float   b;                  // Blue Value
	}
	particle;

	GLfloat color[3] = { 49.0f, 79.0f, 79.0f };
	float   slowdown = 10.0f;          // Slow Down Particles
	float   zoom = -30.0f;            // Used To Zoom Out
	
public:

	SmokeEmitter(int nr, float x, float y , float z );
	~SmokeEmitter();

	bool LoadedGLTextures();
	void DrawParticles();

private:

	bool LoadBMP(const char* FilePath, std::vector<unsigned char> &Pixels);
	bool LoadGLTextures();

	bool m_loadedTexture;
	std::vector<particle*> m_particles;          // Particle Array (Room For Particle Info)
	GLuint  texture[1];         // Storage For Our Particle Texture
	float m_coordX;
	float m_coordY;
	float m_coordZ;
	int m_nrParticles;
};
