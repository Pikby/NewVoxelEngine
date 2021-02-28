#define GLM_FORCE_AVX
#include <glm/glm.hpp>

static const int p[512] = { 151,160,137,91,90,15,                 // Hash lookup table as defined by Ken Perlin.  This is a randomly
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,    // arranged array of all numbers from 0-255 inclusive.
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,151,160,137,91,90,15,                 
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,    
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

class PerlinNoise {
private:
    inline glm::dvec3 fade(const glm::dvec3& t) {
        return t * t * t * (t * (t * glm::dvec3(6) - glm::dvec3(15)) + glm::dvec3(10));
    }

    inline double grad(int hash, const glm::dvec3& pos) {
        double x = pos.x;
        double y = pos.y;
        double z = pos.z;
        switch (hash & 0xF) {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xA: return  y - z;
        case 0xB: return -y - z;
        case 0xC: return  y + x;
        case 0xD: return -y + z;
        case 0xE: return  y - x;
        case 0xF: return -y - z;
        default: return 0;
        }
    }

    inline double lerp(double a, double b, double x) {
        return a + x * (b - a);
    }
public:

    double octavePerlin(const glm::dvec3& pos, int octaves, double persistence) {
        double total = 0;
        double frequency = 1;
        double amplitude = 1;
        double maxValue = 0;  // Used for normalizing result to 0.0 - 1.0
        for (int i = 0; i < octaves; i++) {
            total += perlin(pos * frequency) * amplitude;

            maxValue += amplitude;

            amplitude *= persistence;
            frequency *= 2;
        }

        return total / maxValue;
    }
    

	double perlin(const glm::vec3& pos) {
		glm::ivec3 posi = glm::ivec3(int(glm::floor(pos.x)) & 255, int(glm::floor(pos.y)) & 255, int(glm::floor(pos.z)) & 255);
		glm::dvec3 posd = pos - glm::floor(pos);

		glm::dvec3 faded = fade(posd);

		static const glm::ivec3 table[8] = { {0,0,0},{0,1,0},{0,0,1},{0,1,1}, {1,0,0},{1,1,0},{1,0,1},{1,1,1} };
		
		int hashes[8];
		for (int i = 0; i < 8; i++) {
			hashes[i] = p[p[p[posi.x + table[i].x] + posi.y + table[i].y] + posi.z + table[i].z];
		}

        double x1, x2, y1, y2;
        x1 = lerp(grad(hashes[0], posd), grad(hashes[4], posd-glm::dvec3(table[4])), faded.x);
        x2 = lerp(grad(hashes[1], posd -glm::dvec3(table[1])), grad(hashes[5], posd-glm::dvec3(table[5])), faded.x);
        y1 = lerp(x1, x2, faded.y);

        x1 = lerp(grad(hashes[2], posd - glm::dvec3(table[2])), grad(hashes[6], posd - glm::dvec3(table[6])), faded.x);
        x2 = lerp(grad(hashes[3], posd - glm::dvec3(table[3])), grad(hashes[7], posd - glm::dvec3(table[7])), faded.x);
        y2 = lerp(x1, x2, faded.y);

        double ret = (lerp(y1, y2, faded.z) + 1) / 2.0f;
        return ret;
	}


};

