WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = 
CFLAGS = 
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = 

INC_LINUX_RELEASE =  $(INC) -Iff/ifc
CFLAGS_LINUX_RELEASE =  $(CFLAGS) -O2  -Wall -g -fexceptions -fpermissive -m32 -ffast-math -fno-operator-names  -DLINUX -DNDEBUG -D_JK2EXE -D_IMMERSION_DISABLE -D_FF_DISABLE -D_M_IX86
RESINC_LINUX_RELEASE =  $(RESINC)
RCFLAGS_LINUX_RELEASE =  $(RCFLAGS)
LIBDIR_LINUX_RELEASE =  $(LIBDIR)
LIB_LINUX_RELEASE = $(LIB) -ljpeg -ldl -lopenal -lGL -lX11 -lXxf86vm -lXxf86dga -lXrandr  -pthread -LlibXxf86dga
LDFLAGS_LINUX_RELEASE =  $(LDFLAGS) -m32
OBJDIR_LINUX_RELEASE = ./Release/exe
DEP_LINUX_RELEASE = 
OUT_LINUX_RELEASE = ./Release/jk2sp

OBJ_LINUX_RELEASE = $(OBJDIR_LINUX_RELEASE)/renderer/tr_backend.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_jpeg_interface.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_init.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_image.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_ghoul2.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_font.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_draw.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_curve.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_cmds.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_bsp.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_animation.o $(OBJDIR_LINUX_RELEASE)/renderer/matcomp.o $(OBJDIR_LINUX_RELEASE)/qcommon/unzip.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_scene.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_surfacesprites.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_surface.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_stl.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_sky.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_shadows.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_shader.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_shade_calc.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_shade.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_quicksprite.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_noise.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_model.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_mesh.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_marks.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_main.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_light.o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_load.o $(OBJDIR_LINUX_RELEASE)/mp3code/wavep.o $(OBJDIR_LINUX_RELEASE)/mp3code/upsf.o $(OBJDIR_LINUX_RELEASE)/mp3code/uph.o $(OBJDIR_LINUX_RELEASE)/mp3code/towave.o $(OBJDIR_LINUX_RELEASE)/mp3code/msis.o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_patch.o $(OBJDIR_LINUX_RELEASE)/mp3code/mhead.o $(OBJDIR_LINUX_RELEASE)/mp3code/mdct.o $(OBJDIR_LINUX_RELEASE)/mp3code/l3init.o $(OBJDIR_LINUX_RELEASE)/mp3code/l3dq.o $(OBJDIR_LINUX_RELEASE)/mp3code/hwin.o $(OBJDIR_LINUX_RELEASE)/mp3code/cwinm.o $(OBJDIR_LINUX_RELEASE)/mp3code/cwinb.o $(OBJDIR_LINUX_RELEASE)/qcommon/hstring.o $(OBJDIR_LINUX_RELEASE)/qcommon/strip.o $(OBJDIR_LINUX_RELEASE)/qcommon/net_chan.o $(OBJDIR_LINUX_RELEASE)/qcommon/msg.o $(OBJDIR_LINUX_RELEASE)/qcommon/md4.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_world.o $(OBJDIR_LINUX_RELEASE)/qcommon/files.o $(OBJDIR_LINUX_RELEASE)/qcommon/cvar.o $(OBJDIR_LINUX_RELEASE)/qcommon/common.o $(OBJDIR_LINUX_RELEASE)/qcommon/cmd.o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_trace.o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_test.o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_polylib.o $(OBJDIR_LINUX_RELEASE)/server/sv_savegame.o $(OBJDIR_LINUX_RELEASE)/ui/ui_main.o $(OBJDIR_LINUX_RELEASE)/ui/ui_debug.o $(OBJDIR_LINUX_RELEASE)/ui/ui_connect.o $(OBJDIR_LINUX_RELEASE)/ui/ui_atoms.o $(OBJDIR_LINUX_RELEASE)/server/sv_world.o $(OBJDIR_LINUX_RELEASE)/server/sv_snapshot.o $(OBJDIR_LINUX_RELEASE)/server/sv_main.o $(OBJDIR_LINUX_RELEASE)/server/sv_init.o $(OBJDIR_LINUX_RELEASE)/server/sv_game.o $(OBJDIR_LINUX_RELEASE)/server/sv_client.o $(OBJDIR_LINUX_RELEASE)/server/sv_ccmds.o $(OBJDIR_LINUX_RELEASE)/server/exe_headers.o $(OBJDIR_LINUX_RELEASE)/renderer/tr_worldeffects.o $(OBJDIR_LINUX_RELEASE)/mp3code/cwin.o $(OBJDIR_LINUX_RELEASE)/ui/ui_syscalls.o $(OBJDIR_LINUX_RELEASE)/ui/ui_shared.o $(OBJDIR_LINUX_RELEASE)/game/genericparser2.o $(OBJDIR_LINUX_RELEASE)/client/cl_input.o $(OBJDIR_LINUX_RELEASE)/client/cl_ui.o $(OBJDIR_LINUX_RELEASE)/client/cl_scrn.o $(OBJDIR_LINUX_RELEASE)/client/cl_parse.o $(OBJDIR_LINUX_RELEASE)/client/cl_mp3.o $(OBJDIR_LINUX_RELEASE)/client/cl_main.o $(OBJDIR_LINUX_RELEASE)/client/cl_keys.o $(OBJDIR_LINUX_RELEASE)/client/cl_console.o $(OBJDIR_LINUX_RELEASE)/client/cl_cin.o $(OBJDIR_LINUX_RELEASE)/client/cl_cgame.o $(OBJDIR_LINUX_RELEASE)/encryption/sockets.o $(OBJDIR_LINUX_RELEASE)/encryption/cpp_interface.o $(OBJDIR_LINUX_RELEASE)/encryption/buffer.o $(OBJDIR_LINUX_RELEASE)/client/vmachine.o $(OBJDIR_LINUX_RELEASE)/game/q_math.o $(OBJDIR_LINUX_RELEASE)/client/snd_music.o $(OBJDIR_LINUX_RELEASE)/client/snd_mix.o $(OBJDIR_LINUX_RELEASE)/client/snd_mem.o $(OBJDIR_LINUX_RELEASE)/client/snd_dma.o $(OBJDIR_LINUX_RELEASE)/client/snd_ambient.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdtrans.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jmemnobs.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jmemmgr.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jidctflt.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jfdctflt.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jerror.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdsample.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdpostct.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdmaster.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdmarker.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdmainct.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdinput.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdhuff.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jddctmgr.o $(OBJDIR_LINUX_RELEASE)/mp3code/cupl3.o $(OBJDIR_LINUX_RELEASE)/mp3code/cupini.o $(OBJDIR_LINUX_RELEASE)/mp3code/cupL1.o $(OBJDIR_LINUX_RELEASE)/mp3code/cup.o $(OBJDIR_LINUX_RELEASE)/mp3code/csbtb.o $(OBJDIR_LINUX_RELEASE)/mp3code/csbtL3.o $(OBJDIR_LINUX_RELEASE)/mp3code/csbt.o $(OBJDIR_LINUX_RELEASE)/mp3code/cdct.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdcolor.o $(OBJDIR_LINUX_RELEASE)/linux/linux_snddma.o $(OBJDIR_LINUX_RELEASE)/linux/linux_qgl.o $(OBJDIR_LINUX_RELEASE)/linux/linux_main.o $(OBJDIR_LINUX_RELEASE)/linux/linux_glimp.o $(OBJDIR_LINUX_RELEASE)/linux/linux_console.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jutils.o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_misc.o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_surfaces.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcapimin.o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_bones.o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_bolts.o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_API.o $(OBJDIR_LINUX_RELEASE)/game/q_shared.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdcoefct.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdatasrc.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdatadst.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdapistd.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jdapimin.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jctrans.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcsample.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcprepct.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcphuff.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcparam.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcomapi.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcmaster.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcmarker.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcmainct.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcinit.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jchuff.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jcdctmgr.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jccolor.o $(OBJDIR_LINUX_RELEASE)/jpeg-6/jccoefct.o


all: linux_release

clean: clean_linux_release 

before_linux_release: 
	test -d ./Release || mkdir -p ./Release
	test -d $(OBJDIR_LINUX_RELEASE)/renderer || mkdir -p $(OBJDIR_LINUX_RELEASE)/renderer
	test -d $(OBJDIR_LINUX_RELEASE)/qcommon || mkdir -p $(OBJDIR_LINUX_RELEASE)/qcommon
	test -d $(OBJDIR_LINUX_RELEASE)/mp3code || mkdir -p $(OBJDIR_LINUX_RELEASE)/mp3code
	test -d $(OBJDIR_LINUX_RELEASE)/server || mkdir -p $(OBJDIR_LINUX_RELEASE)/server
	test -d $(OBJDIR_LINUX_RELEASE)/ui || mkdir -p $(OBJDIR_LINUX_RELEASE)/ui
	test -d $(OBJDIR_LINUX_RELEASE)/game || mkdir -p $(OBJDIR_LINUX_RELEASE)/game
	test -d $(OBJDIR_LINUX_RELEASE)/client || mkdir -p $(OBJDIR_LINUX_RELEASE)/client
	test -d $(OBJDIR_LINUX_RELEASE)/encryption || mkdir -p $(OBJDIR_LINUX_RELEASE)/encryption
	test -d $(OBJDIR_LINUX_RELEASE)/jpeg-6 || mkdir -p $(OBJDIR_LINUX_RELEASE)/jpeg-6
	test -d $(OBJDIR_LINUX_RELEASE)/linux || mkdir -p $(OBJDIR_LINUX_RELEASE)/linux
	test -d $(OBJDIR_LINUX_RELEASE)/ghoul2 || mkdir -p $(OBJDIR_LINUX_RELEASE)/ghoul2

after_linux_release: 

linux_release: before_linux_release out_linux_release after_linux_release

out_linux_release: before_linux_release $(OBJ_LINUX_RELEASE) $(DEP_LINUX_RELEASE)
	$(LD) $(LIBDIR_LINUX_RELEASE) -o $(OUT_LINUX_RELEASE) $(OBJ_LINUX_RELEASE)  $(LDFLAGS_LINUX_RELEASE) $(LIB_LINUX_RELEASE)

$(OBJDIR_LINUX_RELEASE)/renderer/tr_backend.o: renderer/tr_backend.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_backend.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_backend.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_jpeg_interface.o: renderer/tr_jpeg_interface.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_jpeg_interface.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_jpeg_interface.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_init.o: renderer/tr_init.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_init.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_init.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_image.o: renderer/tr_image.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_image.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_image.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_ghoul2.o: renderer/tr_ghoul2.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_ghoul2.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_ghoul2.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_font.o: renderer/tr_font.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_font.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_font.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_draw.o: renderer/tr_draw.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_draw.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_draw.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_curve.o: renderer/tr_curve.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_curve.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_curve.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_cmds.o: renderer/tr_cmds.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_cmds.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_cmds.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_bsp.o: renderer/tr_bsp.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_bsp.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_bsp.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_animation.o: renderer/tr_animation.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_animation.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_animation.o

$(OBJDIR_LINUX_RELEASE)/renderer/matcomp.o: renderer/matcomp.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/matcomp.c -o $(OBJDIR_LINUX_RELEASE)/renderer/matcomp.o

$(OBJDIR_LINUX_RELEASE)/qcommon/unzip.o: qcommon/unzip.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/unzip.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/unzip.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_scene.o: renderer/tr_scene.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_scene.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_scene.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_surfacesprites.o: renderer/tr_surfacesprites.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_surfacesprites.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_surfacesprites.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_surface.o: renderer/tr_surface.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_surface.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_surface.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_stl.o: renderer/tr_stl.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_stl.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_stl.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_sky.o: renderer/tr_sky.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_sky.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_sky.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_shadows.o: renderer/tr_shadows.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_shadows.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_shadows.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_shader.o: renderer/tr_shader.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_shader.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_shader.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_shade_calc.o: renderer/tr_shade_calc.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_shade_calc.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_shade_calc.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_shade.o: renderer/tr_shade.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_shade.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_shade.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_quicksprite.o: renderer/tr_quicksprite.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_quicksprite.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_quicksprite.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_noise.o: renderer/tr_noise.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_noise.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_noise.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_model.o: renderer/tr_model.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_model.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_model.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_mesh.o: renderer/tr_mesh.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_mesh.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_mesh.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_marks.o: renderer/tr_marks.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_marks.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_marks.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_main.o: renderer/tr_main.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_main.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_main.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_light.o: renderer/tr_light.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_light.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_light.o

$(OBJDIR_LINUX_RELEASE)/qcommon/cm_load.o: qcommon/cm_load.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/cm_load.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_load.o

$(OBJDIR_LINUX_RELEASE)/mp3code/wavep.o: mp3code/wavep.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/wavep.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/wavep.o

$(OBJDIR_LINUX_RELEASE)/mp3code/upsf.o: mp3code/upsf.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/upsf.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/upsf.o

$(OBJDIR_LINUX_RELEASE)/mp3code/uph.o: mp3code/uph.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/uph.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/uph.o

$(OBJDIR_LINUX_RELEASE)/mp3code/towave.o: mp3code/towave.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/towave.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/towave.o

$(OBJDIR_LINUX_RELEASE)/mp3code/msis.o: mp3code/msis.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/msis.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/msis.o

$(OBJDIR_LINUX_RELEASE)/qcommon/cm_patch.o: qcommon/cm_patch.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/cm_patch.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_patch.o

$(OBJDIR_LINUX_RELEASE)/mp3code/mhead.o: mp3code/mhead.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/mhead.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/mhead.o

$(OBJDIR_LINUX_RELEASE)/mp3code/mdct.o: mp3code/mdct.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/mdct.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/mdct.o

$(OBJDIR_LINUX_RELEASE)/mp3code/l3init.o: mp3code/l3init.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/l3init.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/l3init.o

$(OBJDIR_LINUX_RELEASE)/mp3code/l3dq.o: mp3code/l3dq.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/l3dq.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/l3dq.o

$(OBJDIR_LINUX_RELEASE)/mp3code/hwin.o: mp3code/hwin.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/hwin.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/hwin.o

$(OBJDIR_LINUX_RELEASE)/mp3code/cwinm.o: mp3code/cwinm.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/cwinm.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/cwinm.o

$(OBJDIR_LINUX_RELEASE)/mp3code/cwinb.o: mp3code/cwinb.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/cwinb.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/cwinb.o

$(OBJDIR_LINUX_RELEASE)/qcommon/hstring.o: qcommon/hstring.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/hstring.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/hstring.o

$(OBJDIR_LINUX_RELEASE)/qcommon/strip.o: qcommon/strip.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/strip.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/strip.o

$(OBJDIR_LINUX_RELEASE)/qcommon/net_chan.o: qcommon/net_chan.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/net_chan.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/net_chan.o

$(OBJDIR_LINUX_RELEASE)/qcommon/msg.o: qcommon/msg.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/msg.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/msg.o

$(OBJDIR_LINUX_RELEASE)/qcommon/md4.o: qcommon/md4.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/md4.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/md4.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_world.o: renderer/tr_world.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_world.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_world.o

$(OBJDIR_LINUX_RELEASE)/qcommon/files.o: qcommon/files.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/files.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/files.o

$(OBJDIR_LINUX_RELEASE)/qcommon/cvar.o: qcommon/cvar.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/cvar.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/cvar.o

$(OBJDIR_LINUX_RELEASE)/qcommon/common.o: qcommon/common.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/common.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/common.o

$(OBJDIR_LINUX_RELEASE)/qcommon/cmd.o: qcommon/cmd.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/cmd.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/cmd.o

$(OBJDIR_LINUX_RELEASE)/qcommon/cm_trace.o: qcommon/cm_trace.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/cm_trace.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_trace.o

$(OBJDIR_LINUX_RELEASE)/qcommon/cm_test.o: qcommon/cm_test.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/cm_test.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_test.o

$(OBJDIR_LINUX_RELEASE)/qcommon/cm_polylib.o: qcommon/cm_polylib.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c qcommon/cm_polylib.cpp -o $(OBJDIR_LINUX_RELEASE)/qcommon/cm_polylib.o

$(OBJDIR_LINUX_RELEASE)/server/sv_savegame.o: server/sv_savegame.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/sv_savegame.cpp -o $(OBJDIR_LINUX_RELEASE)/server/sv_savegame.o

$(OBJDIR_LINUX_RELEASE)/ui/ui_main.o: ui/ui_main.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ui/ui_main.cpp -o $(OBJDIR_LINUX_RELEASE)/ui/ui_main.o

$(OBJDIR_LINUX_RELEASE)/ui/ui_debug.o: ui/ui_debug.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ui/ui_debug.cpp -o $(OBJDIR_LINUX_RELEASE)/ui/ui_debug.o

$(OBJDIR_LINUX_RELEASE)/ui/ui_connect.o: ui/ui_connect.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ui/ui_connect.cpp -o $(OBJDIR_LINUX_RELEASE)/ui/ui_connect.o

$(OBJDIR_LINUX_RELEASE)/ui/ui_atoms.o: ui/ui_atoms.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ui/ui_atoms.cpp -o $(OBJDIR_LINUX_RELEASE)/ui/ui_atoms.o

$(OBJDIR_LINUX_RELEASE)/server/sv_world.o: server/sv_world.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/sv_world.cpp -o $(OBJDIR_LINUX_RELEASE)/server/sv_world.o

$(OBJDIR_LINUX_RELEASE)/server/sv_snapshot.o: server/sv_snapshot.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/sv_snapshot.cpp -o $(OBJDIR_LINUX_RELEASE)/server/sv_snapshot.o

$(OBJDIR_LINUX_RELEASE)/server/sv_main.o: server/sv_main.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/sv_main.cpp -o $(OBJDIR_LINUX_RELEASE)/server/sv_main.o

$(OBJDIR_LINUX_RELEASE)/server/sv_init.o: server/sv_init.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/sv_init.cpp -o $(OBJDIR_LINUX_RELEASE)/server/sv_init.o

$(OBJDIR_LINUX_RELEASE)/server/sv_game.o: server/sv_game.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/sv_game.cpp -o $(OBJDIR_LINUX_RELEASE)/server/sv_game.o

$(OBJDIR_LINUX_RELEASE)/server/sv_client.o: server/sv_client.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/sv_client.cpp -o $(OBJDIR_LINUX_RELEASE)/server/sv_client.o

$(OBJDIR_LINUX_RELEASE)/server/sv_ccmds.o: server/sv_ccmds.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/sv_ccmds.cpp -o $(OBJDIR_LINUX_RELEASE)/server/sv_ccmds.o

$(OBJDIR_LINUX_RELEASE)/server/exe_headers.o: server/exe_headers.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c server/exe_headers.cpp -o $(OBJDIR_LINUX_RELEASE)/server/exe_headers.o

$(OBJDIR_LINUX_RELEASE)/renderer/tr_worldeffects.o: renderer/tr_worldeffects.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c renderer/tr_worldeffects.cpp -o $(OBJDIR_LINUX_RELEASE)/renderer/tr_worldeffects.o

$(OBJDIR_LINUX_RELEASE)/mp3code/cwin.o: mp3code/cwin.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/cwin.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/cwin.o

$(OBJDIR_LINUX_RELEASE)/ui/ui_syscalls.o: ui/ui_syscalls.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ui/ui_syscalls.cpp -o $(OBJDIR_LINUX_RELEASE)/ui/ui_syscalls.o

$(OBJDIR_LINUX_RELEASE)/ui/ui_shared.o: ui/ui_shared.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ui/ui_shared.cpp -o $(OBJDIR_LINUX_RELEASE)/ui/ui_shared.o

$(OBJDIR_LINUX_RELEASE)/game/genericparser2.o: game/genericparser2.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c game/genericparser2.cpp -o $(OBJDIR_LINUX_RELEASE)/game/genericparser2.o

$(OBJDIR_LINUX_RELEASE)/client/cl_input.o: client/cl_input.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_input.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_input.o

$(OBJDIR_LINUX_RELEASE)/client/cl_ui.o: client/cl_ui.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_ui.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_ui.o

$(OBJDIR_LINUX_RELEASE)/client/cl_scrn.o: client/cl_scrn.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_scrn.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_scrn.o

$(OBJDIR_LINUX_RELEASE)/client/cl_parse.o: client/cl_parse.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_parse.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_parse.o

$(OBJDIR_LINUX_RELEASE)/client/cl_mp3.o: client/cl_mp3.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_mp3.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_mp3.o

$(OBJDIR_LINUX_RELEASE)/client/cl_main.o: client/cl_main.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_main.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_main.o

$(OBJDIR_LINUX_RELEASE)/client/cl_keys.o: client/cl_keys.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_keys.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_keys.o

$(OBJDIR_LINUX_RELEASE)/client/cl_console.o: client/cl_console.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_console.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_console.o

$(OBJDIR_LINUX_RELEASE)/client/cl_cin.o: client/cl_cin.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_cin.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_cin.o

$(OBJDIR_LINUX_RELEASE)/client/cl_cgame.o: client/cl_cgame.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/cl_cgame.cpp -o $(OBJDIR_LINUX_RELEASE)/client/cl_cgame.o

$(OBJDIR_LINUX_RELEASE)/encryption/sockets.o: encryption/sockets.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c encryption/sockets.cpp -o $(OBJDIR_LINUX_RELEASE)/encryption/sockets.o

$(OBJDIR_LINUX_RELEASE)/encryption/cpp_interface.o: encryption/cpp_interface.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c encryption/cpp_interface.cpp -o $(OBJDIR_LINUX_RELEASE)/encryption/cpp_interface.o

$(OBJDIR_LINUX_RELEASE)/encryption/buffer.o: encryption/buffer.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c encryption/buffer.cpp -o $(OBJDIR_LINUX_RELEASE)/encryption/buffer.o

$(OBJDIR_LINUX_RELEASE)/client/vmachine.o: client/vmachine.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/vmachine.cpp -o $(OBJDIR_LINUX_RELEASE)/client/vmachine.o

$(OBJDIR_LINUX_RELEASE)/game/q_math.o: game/q_math.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c game/q_math.cpp -o $(OBJDIR_LINUX_RELEASE)/game/q_math.o

$(OBJDIR_LINUX_RELEASE)/client/snd_music.o: client/snd_music.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/snd_music.cpp -o $(OBJDIR_LINUX_RELEASE)/client/snd_music.o

$(OBJDIR_LINUX_RELEASE)/client/snd_mix.o: client/snd_mix.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/snd_mix.cpp -o $(OBJDIR_LINUX_RELEASE)/client/snd_mix.o

$(OBJDIR_LINUX_RELEASE)/client/snd_mem.o: client/snd_mem.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/snd_mem.cpp -o $(OBJDIR_LINUX_RELEASE)/client/snd_mem.o

$(OBJDIR_LINUX_RELEASE)/client/snd_dma.o: client/snd_dma.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/snd_dma.cpp -o $(OBJDIR_LINUX_RELEASE)/client/snd_dma.o

$(OBJDIR_LINUX_RELEASE)/client/snd_ambient.o: client/snd_ambient.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c client/snd_ambient.cpp -o $(OBJDIR_LINUX_RELEASE)/client/snd_ambient.o

$(OBJDIR_LINUX_RELEASE)/mp3code/cupl3.o: mp3code/cupl3.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/cupl3.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/cupl3.o

$(OBJDIR_LINUX_RELEASE)/mp3code/cupini.o: mp3code/cupini.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/cupini.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/cupini.o

$(OBJDIR_LINUX_RELEASE)/mp3code/cupL1.o: mp3code/cupL1.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/cupL1.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/cupL1.o

$(OBJDIR_LINUX_RELEASE)/mp3code/cup.o: mp3code/cup.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/cup.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/cup.o

$(OBJDIR_LINUX_RELEASE)/mp3code/csbtb.o: mp3code/csbtb.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/csbtb.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/csbtb.o

$(OBJDIR_LINUX_RELEASE)/mp3code/csbtL3.o: mp3code/csbtL3.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/csbtL3.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/csbtL3.o

$(OBJDIR_LINUX_RELEASE)/mp3code/csbt.o: mp3code/csbt.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/csbt.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/csbt.o

$(OBJDIR_LINUX_RELEASE)/mp3code/cdct.o: mp3code/cdct.c
	$(CC) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c mp3code/cdct.c -o $(OBJDIR_LINUX_RELEASE)/mp3code/cdct.o

$(OBJDIR_LINUX_RELEASE)/linux/linux_snddma.o: linux/linux_snddma.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c linux/linux_snddma.cpp -o $(OBJDIR_LINUX_RELEASE)/linux/linux_snddma.o

$(OBJDIR_LINUX_RELEASE)/linux/linux_qgl.o: linux/linux_qgl.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c linux/linux_qgl.cpp -o $(OBJDIR_LINUX_RELEASE)/linux/linux_qgl.o

$(OBJDIR_LINUX_RELEASE)/linux/linux_main.o: linux/linux_main.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c linux/linux_main.cpp -o $(OBJDIR_LINUX_RELEASE)/linux/linux_main.o

$(OBJDIR_LINUX_RELEASE)/linux/linux_glimp.o: linux/linux_glimp.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c linux/linux_glimp.cpp -o $(OBJDIR_LINUX_RELEASE)/linux/linux_glimp.o

$(OBJDIR_LINUX_RELEASE)/linux/linux_console.o: linux/linux_console.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c linux/linux_console.cpp -o $(OBJDIR_LINUX_RELEASE)/linux/linux_console.o

$(OBJDIR_LINUX_RELEASE)/ghoul2/G2_misc.o: ghoul2/G2_misc.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ghoul2/G2_misc.cpp -o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_misc.o

$(OBJDIR_LINUX_RELEASE)/ghoul2/G2_surfaces.o: ghoul2/G2_surfaces.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ghoul2/G2_surfaces.cpp -o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_surfaces.o

$(OBJDIR_LINUX_RELEASE)/ghoul2/G2_bones.o: ghoul2/G2_bones.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ghoul2/G2_bones.cpp -o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_bones.o

$(OBJDIR_LINUX_RELEASE)/ghoul2/G2_bolts.o: ghoul2/G2_bolts.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ghoul2/G2_bolts.cpp -o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_bolts.o

$(OBJDIR_LINUX_RELEASE)/ghoul2/G2_API.o: ghoul2/G2_API.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c ghoul2/G2_API.cpp -o $(OBJDIR_LINUX_RELEASE)/ghoul2/G2_API.o

$(OBJDIR_LINUX_RELEASE)/game/q_shared.o: game/q_shared.cpp
	$(CXX) $(CFLAGS_LINUX_RELEASE) $(INC_LINUX_RELEASE) -c game/q_shared.cpp -o $(OBJDIR_LINUX_RELEASE)/game/q_shared.o

clean_linux_release: 
	rm -f $(OBJ_LINUX_RELEASE) $(OUT_LINUX_RELEASE)
	rm -rf ./Release
	rm -rf $(OBJDIR_LINUX_RELEASE)/renderer
	rm -rf $(OBJDIR_LINUX_RELEASE)/qcommon
	rm -rf $(OBJDIR_LINUX_RELEASE)/mp3code
	rm -rf $(OBJDIR_LINUX_RELEASE)/server
	rm -rf $(OBJDIR_LINUX_RELEASE)/ui
	rm -rf $(OBJDIR_LINUX_RELEASE)/game
	rm -rf $(OBJDIR_LINUX_RELEASE)/client
	rm -rf $(OBJDIR_LINUX_RELEASE)/encryption
	rm -rf $(OBJDIR_LINUX_RELEASE)/jpeg-6
	rm -rf $(OBJDIR_LINUX_RELEASE)/linux
	rm -rf $(OBJDIR_LINUX_RELEASE)/ghoul2


.PHONY: before_linux_release after_linux_release clean_linux_release 
