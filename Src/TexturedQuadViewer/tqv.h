#pragma once

class AppContext : private Uncopyable
{
public:
	int										windowWidth;
	int										windowHeight;
	char									bHoldingKeys[SDLK_LAST];

	ArnTexture*								texture;
	std::vector<std::pair<double,double> >	quadVertices;
	int										gridCountX;
	int										gridCountY;
	std::string								inputFileName;
};
