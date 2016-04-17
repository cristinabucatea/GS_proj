#include "SmokeEmitter.h"

SmokeEmitter::SmokeEmitter(int nr, float x = 0, float y = 0, float z = 0) :
	m_nrParticles(nr), m_coordX(x), m_coordY(y), m_coordZ(z)
{
	m_loadedTexture = LoadGLTextures();
	glEnable(GL_TEXTURE_2D);                        // Enable Texture Mapping
	for (int i = 0; i < nr; i++)                   // Initialize All The Textures
	{
		m_particles.push_back(new particle());
		m_particles[i]->active = true;                 // Make All The Particles Active
		m_particles[i]->life = 1.0f;                   // Give All The Particles Life
		m_particles[i]->fade = float(rand() % 100) / 1000.0f + 0.001f;       // Random Fade Speed
		m_particles[i]->r = color[0];
		m_particles[i]->g = color[1];
		m_particles[i]->b = color[2];
		m_particles[i]->x = m_coordX;                  // Center On X Axis
		m_particles[i]->y = m_coordY;                  // Center On Y Axis
		m_particles[i]->z = m_coordZ;                  // Center On Z Axis
		m_particles[i]->xi = 0.0f;       // Random Speed On X Axis
		m_particles[i]->yi = 0.0f;       // Random Speed On Y Axis
		m_particles[i]->zi = 0.0f;       // Random Speed On Z Axis
	}
	glBindTexture(GL_TEXTURE_2D, texture[0]);                // Select Our Texture
}

SmokeEmitter::~SmokeEmitter()
{
	m_particles.clear();
}

bool SmokeEmitter::LoadedGLTextures()
{
	return m_loadedTexture;
}

void SmokeEmitter::DrawParticles()
{
	// Reset The ModelView Matrix									// Reset The Current Modelview Matrix
	for (int i = 0; i < m_nrParticles; i++)                   // i Through All The Particles
	{
		if (m_particles[i]->active)                  // If The Particle Is Active
		{
			float x = m_particles[i]->x;               // Grab Our Particle X Position
			float y = m_particles[i]->y;               // Grab Our Particle Y Position
													   //TODO: Here remove the zoom when adding it to new scene
			float z = m_particles[i]->z + zoom;              // Particle Z Pos + Zoom
															 // Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(m_particles[i]->r, m_particles[i]->g, m_particles[i]->b, m_particles[i]->life);
			glBegin(GL_TRIANGLE_STRIP);             // Build Quad From A Triangle Strip
			glTexCoord2d(1, 1);
			glVertex3f(x + 0.5f, y + 0.5f, z); // Top Right
			glTexCoord2d(0, 1);
			glVertex3f(x - 0.5f, y + 0.5f, z); // Top Left
			glTexCoord2d(1, 0);
			glVertex3f(x + 0.5f, y - 0.5f, z); // Bottom Right
			glTexCoord2d(0, 0);
			glVertex3f(x - 0.5f, y - 0.5f, z); // Bottom Left
			glEnd();                        // Done Building Triangle Strip

			m_particles[i]->x += m_particles[i]->xi / (slowdown * 800);    // Move On The X Axis By X Speed
			m_particles[i]->y += m_particles[i]->yi / (slowdown * 800);    // Move On The Y Axis By Y Speed
			m_particles[i]->z += m_particles[i]->zi / (slowdown * 800);    // Move On The Z Axis By Z Speed

			m_particles[i]->life -= m_particles[i]->fade;       // Reduce Particles Life By 'Fade'
			if (m_particles[i]->life < 0.0f)                    // If Particle Is Burned Out
			{
				m_particles[i]->life = 1.0f;               // Give It New Life
				m_particles[i]->fade = float(rand() % 100) / 1000.0f + 0.001f;   // Random Fade Value
				m_particles[i]->x = m_coordX;                  // Center On X Axis
				m_particles[i]->y = m_coordY;                  // Center On Y Axis
				m_particles[i]->z = m_coordZ;                  // Center On Z Axis
				m_particles[i]->xi = float((rand() % 60) - 30.0f);  // X Axis Speed And Direction
				m_particles[i]->yi = float((rand() % 60) + 30.0f);  // Y Axis Speed And Direction
				m_particles[i]->zi = float((rand() % 60) + 30.0f);     // Z Axis Speed And Direction
			}
		}
	}
}

bool SmokeEmitter::LoadBMP(const char* FilePath, std::vector<unsigned char> &Pixels)
{
	int width = 0;
	int height = 0;
	short BitsPerPixel = 0;

	std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
	if (!hFile.is_open())
		return false;

	hFile.seekg(0, std::ios::end);
	int Length = hFile.tellg();
	hFile.seekg(0, std::ios::beg);
	std::vector<std::uint8_t> FileInfo(Length);
	hFile.read(reinterpret_cast<char*>(FileInfo.data()), 54);

	if (FileInfo[0] != 'B' && FileInfo[1] != 'M')
	{
		hFile.close();
		return false;
	}

	if (FileInfo[28] != 24 && FileInfo[28] != 32)
	{
		hFile.close();
		return false;
	}

	BitsPerPixel = FileInfo[28];
	width = FileInfo[18] + (FileInfo[19] << 8);
	height = FileInfo[22] + (FileInfo[23] << 8);
	std::uint32_t PixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
	std::uint32_t size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
	Pixels.resize(size);

	hFile.seekg(PixelsOffset, std::ios::beg);
	hFile.read(reinterpret_cast<char*>(Pixels.data()), size);
	hFile.close();

	return true;
}

bool SmokeEmitter::LoadGLTextures() // Load Bitmaps And Convert To Textures
{
	bool Status = FALSE; // Status Indicator
	std::vector<unsigned char> Pixels;
	if (LoadBMP("Particle.bmp", Pixels)) // Load Particle Texture
	{
		Status = TRUE; // Set The Status To TRUE
		glGenTextures(1, &texture[0]); // Create One Textures

		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, Pixels.data());
	}

	return Status; // Return The Status
}