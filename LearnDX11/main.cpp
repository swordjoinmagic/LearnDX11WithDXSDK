// LearnDX11.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <windows.h>
#include <iostream>
#include "InitDirect3DApp.h"
#include "Sample1_Cube.h"
#include "Sample2_SkyBox.h"
#include "Sample3_Light.h"
#include "Sample4_PointShadow.h"
#include "Sample5_Deffered.h"
#include "Sample6_QuadTree.h"
#include "Sample7_CSM.h"
#include "Sample8_ShadowTest.h"
#include "Sample9_ParticleSystem.h"
#include "Sample10_StreamOutTest.h"
#include "Sample11_CascadedShadowMap.h"
#include "Sample12_FrustumWithQuadTree.h"
#include "Sample13_Animation.h"

int main()
{
	HINSTANCE hInstance = GetModuleHandle(0);
	Sample13 sample(hInstance);
	if (!sample.Init()) {
		std::cout << "应用程序初始化错误!!!" << std::endl;
	}
	return sample.Run();
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
