# -*- coding: UTF-8 -*-
#!/bin/python

import wx
import os
import sys, platform
import skyeye_ir
import x

os_info = platform.system()
if cmp(os_info, "Linux"):
	print "In Windows OS"
	from skyeye_common_windows_module import *
	import windows_font as C
else:
	print "In Linux OS"
	from skyeye_common_module import *
	import linux_font as C
from ctypes import *


def GetImage(self, picture, l, h):
	image = wx.Image(picture)
	image.Rescale(l, h)
	imageBit = wx.BitmapFromImage(image)
	return imageBit

OpenID = wx.NewId()
ChpID = wx.NewId()
RunID = wx.NewId()
StopID = wx.NewId()
DebugID = wx.NewId()
SetID = wx.NewId()
SetID = wx.NewId()
HelpID = wx.NewId()

class MainFrame(wx.Frame):
	InfoRegsDlg = None
	def __init__(self, parent = None, id = -1,
			title = C.FontMainTitle):
		wx.Frame.__init__(self, parent, id, title, size = (450, 300))
		self.MenuBar = wx.MenuBar()
		# Create Menu for Debug
		self.DebugMenu = wx.Menu()
		Registers = wx.MenuItem(self.DebugMenu, wx.NewId(), C.FontInfoRegs)
		Memories = wx.MenuItem(self.DebugMenu, wx.NewId(), C.FontShowMems)
		RemoteGdb = wx.MenuItem(self.DebugMenu, wx.NewId(), C.FontRemoteGdb)
		self.DebugMenu.AppendItem(Registers)
		self.DebugMenu.AppendItem(Memories)
		self.DebugMenu.AppendItem(RemoteGdb)
		self.Bind(wx.EVT_MENU, self.InfoRegs, id=Registers.GetId())
		self.Bind(wx.EVT_MENU, self.ShowMem, id=Memories.GetId())
		self.Bind(wx.EVT_MENU, self.RemoteGdb, id=RemoteGdb.GetId())
			
		# Create Menu for File
		self.FileMenu = wx.Menu()
		OpenConf = wx.MenuItem(self.FileMenu, wx.NewId(), C.FontOpenConfig)
		OpenChp = wx.MenuItem(self.FileMenu, wx.NewId(), C.FontOpenSnapshot)
		SaveChp = wx.MenuItem(self.FileMenu, wx.NewId(), C.FontSaveSnapshot)
		Quit = wx.MenuItem(self.FileMenu, wx.NewId(), C.FontQuit)
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
		UserMenu = wx.MenuItem(self.HelpMenu, wx.NewId(), C.FontUserManu)
		VersionMenu = wx.MenuItem(self.HelpMenu, wx.NewId(), C.FontVersion)
		self.HelpMenu.AppendItem(UserMenu)
		self.HelpMenu.AppendItem(VersionMenu)
		self.Bind(wx.EVT_MENU, self.UserMan, id= UserMenu.GetId())
		self.Bind(wx.EVT_MENU, self.ShowVersion, id= VersionMenu.GetId())

		# Add the Memuitem into  MenuBar
		self.MenuBar.Append(self.FileMenu, C.FontFileMenu)
		self.MenuBar.Append(self.DebugMenu, C.FontDebugMenu)
		self.MenuBar.Append(self.HelpMenu, C.FontHelpMenu)

		# Add the Tool Bar
		self.ImageOpen = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/open.png", 50, 50)
		self.ImageSet = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/set.png", 50, 50)
		self.ImageStop = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/stop.png", 50, 50)
		self.ImageRun = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/run.png", 45, 45)
		self.ImageMore = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/debug.png", 52, 50)
		self.ImageHelp= GetImage(self, os.getenv("SKYEYEBIN") + "./picture/help.png", 40, 40)
		self.ImageChp = GetImage(self, os.getenv("SKYEYEBIN") + "./picture/chp.png", 50, 50)

		self.ToolBar = self.CreateToolBar(wx.TB_HORIZONTAL | wx.NO_BORDER | wx.TB_FLAT 
			        | wx.TB_TEXT | wx.TB_3DBUTTONS)
		
		self.ToolBar.AddSimpleTool(OpenID, self.ImageOpen, C.FontOpenConfig)
		self.ToolBar.AddSimpleTool(ChpID, self.ImageChp, C.FontSaveSnapshot)
		self.ToolBar.AddSeparator()
		# The button for run and stop
		self.ToolBar.AddSimpleTool(RunID, self.ImageRun, C.FontRun)
		self.ToolBar.AddSimpleTool(DebugID, self.ImageMore, C.FontRemoteGdb)
		self.ToolBar.AddSeparator()
		self.ToolBar.AddSimpleTool(SetID, self.ImageSet, C.FontSet)
		self.ToolBar.AddSimpleTool(HelpID, self.ImageHelp, C.FontVersion)
		self.ToolBar.Realize()

		self.Bind(wx.EVT_TOOL, self.OpenConfig, id = OpenID)
		self.Bind(wx.EVT_TOOL, self.SetConfig, id = SetID)
		self.Bind(wx.EVT_TOOL, self.SaveChp, id = ChpID)
		self.Bind(wx.EVT_TOOL, self.Run, id = RunID)
		self.Bind(wx.EVT_TOOL, self.RemoteGdb, id = DebugID)
		self.Bind(wx.EVT_TOOL, self.Help, id = HelpID)

		self.Panel = wx.Panel(self)
		self.Panel.SetBackgroundColour('White')
		wx.StaticText(self.Panel, -1, C.FontCPU, (10, 10))
		self.cpu = wx.StaticText(self.Panel, -1, "NULL", (80, 10))
		wx.StaticText(self.Panel, -1, C.FontStatus, (10, 50))
		self.status = wx.StaticText(self.Panel, -1, "NULL", (80, 50))


	def OpenConfig(self, event):
		dialog = wx.FileDialog(None, C.FontOpenConfig, os.getcwd(),"", "All files (*)|*", wx.OPEN)
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
		self.status.SetLabel(C.FontStop)
		self.cpu.SetLabel("DSP")

	
	def Run(self, event):
		self.ToolBar.DeleteTool(id = RunID)
		self.ToolBar.InsertSimpleTool(3, StopID, self.ImageStop, C.FontStop)
		self.Bind(wx.EVT_TOOL, self.Stop, id = StopID)
		self.ToolBar.Realize()
        	libcommon.SIM_run()
		self.status.SetLabel(C.FontRun)

	def Stop(self, event):
		self.ToolBar.DeleteTool(id = StopID)
		self.ToolBar.InsertSimpleTool(3, RunID, self.ImageRun, C.FontRun)
		self.Bind(wx.EVT_TOOL, self.Run, id = RunID)
		self.ToolBar.Realize()
        	libcommon.SIM_stop()
		self.status.SetLabel(C.FontStop)
		self.RefurbishSubGui()
		
	def SetConfig(self, event):
		print "In SetConfig"
		
	def RemoteGdb(self, event):
		libgdbserver.com_remote_gdb()
		self.status.SetLabel(C.FontRemoteGdb)

	def Help(self, event):
		print "In Help"

	def OpenChp(self, event):
		print "In Open checkpoint"

	def SaveChp(self, event):
		dialog = wx.FileDialog(None, C.FontSaveSnapshot, os.getcwd(),
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
		app = wx.PySimpleApp()
		self.ShowMemFrame = x.Memory()
		self.ShowMemFrame.Show()
		app.MainLoop()


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
		if(self.ShowMem != None):
			self.ShowMemFrame.Refreshput()
		else:
			print "Show Mem is None"


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
