#pragma once

class ArnDx9Dev
{
public:
	static LPDIRECT3DDEVICE9 Get() { return ms_dev; }

private:
	static LPDIRECT3DDEVICE9 ms_dev;
};
