#pragma once



class Renderer
{
public:
	virtual ~Renderer(void);

	virtual void setProjTransform(ArnMatrix& m) = 0;
	virtual void setViewTransform(ArnMatrix& m) = 0;
	virtual void setWorldTransform(ArnMatrix& m) = 0;

private:
	Renderer(void);
};
