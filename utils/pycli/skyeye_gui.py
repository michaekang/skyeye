# -*- coding: UTF-8 -*-
#!/bin/python

import wx
import os
import sys, platform
import skyeye_ir

os_info = platform.system()
if cmp(os_info, "Linux"):
    print "In Windows OS"
    from skyeye_common_windows_module import *
else:
    print "In Linux OS"
    from skyeye_common_module import *
from ctypes import *


def GetImage(self, picture, l, h):
	image = wx.Image(picture)
	image.Rescale(l, h)
	imageBit = wx.BitmapFromImage(image)
	return imageBit

class MainFrame(wx.Frame):
	InfoRegsDlg = None
	def __init__(self, parent = None, id = -1,
			title = "天目仿真平台"):
		wx.Frame.__init__(self, parent, id, title, size = (450, 300))
		self.MenuBar = wx.MenuBar()
		# Create Menu for Debug
		self.DebugMenu = wx.Menu()
		Registers = wx.MenuItem(self.DebugMenu, wx.NewId(), '查看寄存器')
		Memories = wx.MenuItem(self.DebugMenu, wx.NewId(), '查看内存')
		RemoteGdb = wx.MenuItem(self.DebugMenu, wx.NewId(), '远程调试')
		self.DebugMenu.AppendItem(Registers)
		self.DebugMenu.AppendItem(Memories)
		self.DebugMenu.AppendItem(RemoteGdb)
		self.Bind(wx.EVT_MENU, self.InfoRegs, id=Registers.GetId())
		self.Bind(wx.EVT_MENU, self.ShowMem, id=Memories.GetId())
		self.Bind(wx.EVT_MENU, self.RemoteGdb, id=RemoteGdb.GetId())
			
		# Create Menu for File
		self.FileMenu = wx.Menu()
		OpenConf = wx.MenuItem(self.FileMenu, wx.NewId(), '打开配置文件')
		OpenChp = wx.MenuItem(self.FileMenu, wx.NewId(), '恢复快闪状态')
		SaveChp = wx.MenuItem(self.FileMenu, wx.NewId(), '保存快闪状态')
		Quit = wx.MenuItem(self.FileMenu, wx.NewId(), '退出')
		self.FileMenu.AppendItem(OpenConf)
		self.FileMenu.AppendItem(OpenChp)
		self.FileMenu.AppendItem(SaveChp)
		self.FileMenu.AppendItem(Quit)
		self.Bind(wx.EVT_MENU, self.OpenConfig, id=OpenConf.GetId())
		self.Bind(wx.EVT_MENU, self.OpenChp, id=OpenChp.GetId())
		self.Bind(wx.EVT_MENU, self.SaveChp, id=SaveChp.GetId())
		self.Bind(wx.EVT_MENU, self.QuitSky, id=Quit.GetId())

		# Create Menu for Help
		self.HelpMenu = wx.Menu()
		UserMenu = wx.MenuItem(self.HelpMenu, wx.NewId(), '用户手册')
		VersionMenu = wx.MenuItem(self.HelpMenu, wx.NewId(), '版本信息')
		self.HelpMenu.AppendItem(UserMenu)
		self.HelpMenu.AppendItem(VersionMenu)
		self.Bind(wx.EVT_MENU, self.UserMan, id= UserMenu.GetId())
		self.Bind(wx.EVT_MENU, self.ShowVersion, id= VersionMenu.GetId())

		# Add the Memuitem into  MenuBar
		self.MenuBar.Append(self.FileMenu, "文件")
		self.MenuBar.Append(self.DebugMenu, "调试")
		self.MenuBar.Append(self.HelpMenu, "帮助")

		# Add the Tool Bar
		self.ImageOpen = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/open.png", 50, 50)
		self.ImageSet = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/set.png", 50, 50)
		self.ImageStop = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/stop.png", 50, 50)
		self.ImageRun = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/run.png", 45, 45)
		self.ImageMore = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/debug.png", 40, 40)
		self.ImageHelp= GetImage(self, os.getenv("SKYEYEBIN") + "./picture/help.png", 40, 40)
		self.ImageChp = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/chp.png", 50, 50)

		self.ToolBar = self.CreateToolBar(wx.TB_HORIZONTAL | wx.NO_BORDER | wx.TB_FLAT 
			        | wx.TB_TEXT | wx.TB_3DBUTTONS)
		self.ToolBar.AddSimpleTool(1, self.ImageOpen, "打开配置")
		self.ToolBar.AddSimpleTool(3, self.ImageChp, "保存状态快照")
		self.ToolBar.AddSeparator()
		# The button for run and stop
		self.RunBtn = wx.BitmapButton(self.ToolBar, -1, self.ImageRun, style = wx.BU_AUTODRAW, name = "Run")
		self.RunBtn.SetBitmapSelected(self.ImageStop)
		self.ToolBar.AddControl(self.RunBtn)

		self.ToolBar.AddSimpleTool(5, self.ImageMore, "远程调试")
		self.ToolBar.AddSeparator()
		self.ToolBar.AddSimpleTool(2, self.ImageSet, "设置配置文件")
		self.ToolBar.AddSimpleTool(6, self.ImageHelp, "帮助")

		self.Bind(wx.EVT_TOOL, self.OpenConfig, id = 1)
		self.Bind(wx.EVT_TOOL, self.SetConfig, id = 2)
		self.Bind(wx.EVT_TOOL, self.SaveChp, id = 3)
		self.Bind(wx.EVT_BUTTON, self.Run, self.RunBtn)
		self.Bind(wx.EVT_TOOL, self.RemoteGdb, id = 5)
		self.Bind(wx.EVT_TOOL, self.Help, id = 6)

		self.Panel = wx.Panel(self)
		self.Panel.SetBackgroundColour('White')
		wx.StaticText(self.Panel, -1, "处理器:", (10, 10))
		self.cpu = wx.StaticText(self.Panel, -1, "NULL", (80, 10))
		wx.StaticText(self.Panel, -1, "状态 :", (10, 50))
		self.status = wx.StaticText(self.Panel, -1, "NULL", (80, 50))
#		wx.StaticText(self.Panel, -1, "配置文件:", (10, 90))
#		self.SkyeyeConf = wx.StaticText(self.Panel, -1, "NULL", (80, 90))



	def OpenConfig(self, event):
		dialog = wx.FileDialog(None, "选择<配置文件>", os.getcwd(),"", "All files (*)|*", wx.OPEN)
	        if dialog.ShowModal() == wx.ID_OK:
	            configure_file =  dialog.GetPath()
	            last_path = dialog.GetDirectory()
		os.chdir(last_path)
		if configure_file == "":
			dialog.Destroy()
			return
		#change the string of python to string of C.
		libcommon.com_load_conf(c_char_p(configure_file))
        	libcommon.SIM_start()
		dialog.Destroy()
		DspPic = wx.StaticBitmap(self.Panel, -1,  pos=(200,10), size=(300, 300))
		ImageDSP = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/DSP.jpg", 200, 200)
		DspPic.SetBitmap(ImageDSP)
		self.status.SetLabel("停止")
		self.cpu.SetLabel("DSP")

	
	def Run(self, event):
		self.RunBtn.SetBitmapLabel(self.ImageStop)
		self.Bind(wx.EVT_BUTTON, self.Stop, self.RunBtn)
        	libcommon.SIM_run()
		self.status.SetLabel("运行")

	def Stop(self, event):
		self.Bind(wx.EVT_BUTTON, self.Run, self.RunBtn)
		self.RunBtn.SetBitmapLabel(self.ImageRun)
        	libcommon.SIM_stop()
		self.RefurbishSubGui()
		self.status.SetLabel("停止")
		
	def SetConfig(self, event):
		print "In SetConfig"
		
	def RemoteGdb(self, event):
		libgdbserver.com_remote_gdb()
		self.status.SetLabel("远程调试")

	def Help(self, event):
		print "In Help"

	def OpenChp(self, event):
		print "In Open checkpoint"

	def SaveChp(self, event):
		dialog = wx.FileDialog(None, "保存快照", os.getcwd(),
					"", "All files (*)|*", wx.SAVE)
	        if dialog.ShowModal() == wx.ID_OK:
			Checkpoint =  dialog.GetPath()
			last_path = dialog.GetDirectory()
		print Checkpoint, last_path

	def InfoRegs(self, event):
		app = wx.PySimpleApp()
		self.InfoRegsDlg = skyeye_ir.InfoRegsDialog()
		self.InfoRegsDlg.Show()
		app.MainLoop()

	def ShowMem(self, event):
		print "In Info Memory"

	def UserMan(self, event):
		print "In User Manu"

	def ShowVersion(self, event):
		print "In ShowVersion"

	def QuitSky(self, event):
		self.Close(True)
		self.Destroy()
		libcommon.com_quit()

	# Add the SubGui's refurbish in this function, call it when you stop skyeye
	def RefurbishSubGui(self):
		if(self.InfoRegsDlg != None):
			self.InfoRegsDlg.RegsRefurbish()
		else:
			print "Info Regs is None"



class SkyEyeGUI(wx.App):
	def OnInit(self):
		self.frame = MainFrame(parent = None)
		self.frame.Show()
		self.SetTopWindow(self.frame)
		return True
	def OnExit(self):
		libcommon.com_quit()

if __name__ == '__main__':
	app = SkyEyeGUI()
	app.MainLoop()
