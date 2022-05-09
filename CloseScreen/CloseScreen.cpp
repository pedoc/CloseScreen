#include <iostream>
#include <vector>
#include <Windows.h>
#include <WinUser.h>
#include <lowlevelmonitorconfigurationapi.h>

#pragma comment(lib, "Dxva2.lib")


constexpr BYTE PowerMode = 0xD6;
constexpr DWORD PowerOn = 0x01;
constexpr DWORD PowerOff = 0x04;
constexpr BYTE MonitorStateOn = -1;
constexpr BYTE  MonitorStateOff = 2;
constexpr     BYTE MonitorStateStandBy = 1;

struct MonitorDesc
{
	DWORD idx;
	HANDLE hdl;
	HDC hdc;
	DWORD power;
	MONITORINFO info;
	BOOL primary;
};

MONITORINFO GetMonitorInfoEx(HMONITOR hMonitor)
{
	MONITORINFO info = {  };
	info.cbSize = sizeof(info);

	if (GetMonitorInfo(hMonitor, &info))
	{
		std::cout << "显示器 x: " << std::abs(info.rcMonitor.left - info.rcMonitor.right)
			<< " y: " << std::abs(info.rcMonitor.top - info.rcMonitor.bottom)
			<< " flags:" << info.dwFlags
			<< std::endl;
	}
	return info;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	auto pMonitors = reinterpret_cast<std::vector<MonitorDesc>*>(dwData);  // NOLINT(performance-no-int-to-ptr)

	DWORD nMonitorCount;
	if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &nMonitorCount))
	{
		const auto pMons = new PHYSICAL_MONITOR[nMonitorCount];
		const auto info = GetMonitorInfoEx(hMonitor);
		if (GetPhysicalMonitorsFromHMONITOR(hMonitor, nMonitorCount, pMons))
		{
			for (DWORD i = 0; i < nMonitorCount; i++)
			{
				MonitorDesc desc = { };
				desc.hdl = pMons[i].hPhysicalMonitor;
				desc.idx = i;
				desc.power = -1;
				desc.info = info;
				desc.primary = (desc.info.dwFlags & MONITORINFOF_PRIMARY) != 0;
				pMonitors->push_back(desc);
			}
		}
		else
		{
			const DWORD error = GetLastError();
			std::cout << "GetPhysicalMonitorsFromHMONITOR 失败:" << hMonitor << ",错误代码:" << error;
		}
		delete[] pMons;
	}
	return TRUE;
}

BOOL ToggleMonitor(MonitorDesc& monitor)
{
	MC_VCP_CODE_TYPE vcp = {};
	DWORD curr = -1;
	DWORD max = -1;
	GetVCPFeatureAndVCPFeatureReply(monitor.hdl, PowerMode, &vcp, &curr, &max);

	monitor.power = curr;
	const DWORD mode = monitor.power == PowerOff ? PowerOn : PowerOff;
	const BOOL bSuccess = SetVCPFeature(monitor.hdl, PowerMode, mode);
	monitor.power = mode;
	return bSuccess;
}

HMONITOR GetPrimaryMonitorHandle()
{
	// ReSharper disable once CppInconsistentNaming
	const POINT ptZero = { 0, 0 };
	return MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
}

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		std::cout << "关闭所有显示器";
		//SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM) MonitorStateOff);
		PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, MonitorStateOff);
	}
	else
	{
		const int monitorIdx = *argv[1] - '0';
		if (monitorIdx < 0 || monitorIdx>9)
		{
			std::cout << "显示器索引越界";
			return -1;
		}

		std::vector<MonitorDesc> monitors;
		EnumDisplayMonitors(nullptr, nullptr, &MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
		if (monitors.capacity() <= 0 || monitors.capacity() < monitorIdx + 1)
		{
			std::cout << "未检测到显示器或显示器索引超出实际数量";
			return -1;
		}
		if (ToggleMonitor(monitors[monitorIdx]))
			//if (MonitorOff(monitors[monitorIdx].hdc))
		{
			std::cout << "已关闭索引编号为" << monitorIdx << "的显示器";
		}
		else
		{
			const DWORD error = GetLastError();
			std::cout << "尝试关闭索引编号为" << monitorIdx << "的显示器时失败,错误代码:" << error;
		}
	}
}