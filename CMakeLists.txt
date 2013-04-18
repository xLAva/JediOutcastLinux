#
# Build procedure:
#                $ mkdir some_build_dir
#                $ cd some_build_dir
#                $ cmake PATH_TO_JK2_SOURCE_DIR
#                $ make
#
# Make sure some_build_dir is different from PATH_TO_JK2_SOURCE_DIR
#

cmake_minimum_required(VERSION 2.6)

project(jk2)

set(CMAKE_BUILD_TYPE "RELEASE")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-write-strings -Wno-pointer-arith -Wno-multichar -Wno-int-to-pointer-cast -Wno-enum-compare -Wno-cpp -fexceptions -m32 -ffast-math -DLINUX -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-write-strings -Wno-conversion-null -Wno-pointer-arith -Wno-multichar -Wno-int-to-pointer-cast -Wno-enum-compare -Wno-cpp -fexceptions -fpermissive -m32 -ffast-math -fno-operator-names -DLINUX -DNDEBUG")

set(GAME_SOURCE_FILES
	code/cgame/FX_ATSTMain.cpp
	code/cgame/FX_Blaster.cpp
	code/cgame/FX_Bowcaster.cpp
	code/cgame/FX_BryarPistol.cpp
	code/cgame/FX_DEMP2.cpp
	code/cgame/FX_Disruptor.cpp
	code/cgame/FX_Emplaced.cpp
	code/cgame/FX_Flechette.cpp
	code/cgame/FX_HeavyRepeater.cpp
	code/cgame/FX_RocketLauncher.cpp
	code/cgame/FxParsing.cpp
	code/cgame/FxPrimitives.cpp
	code/cgame/FxScheduler.cpp
	code/cgame/FxSystem.cpp
	code/cgame/FxTemplate.cpp
	code/cgame/FxUtil.cpp
	code/cgame/cg_camera.cpp
	code/cgame/cg_consolecmds.cpp
	code/cgame/cg_credits.cpp
	code/cgame/cg_draw.cpp
	code/cgame/cg_drawtools.cpp
	code/cgame/cg_effects.cpp
	code/cgame/cg_ents.cpp
	code/cgame/cg_event.cpp
	code/cgame/cg_headers.cpp
	code/cgame/cg_info.cpp
	code/cgame/cg_lights.cpp
	code/cgame/cg_localents.cpp
	code/cgame/cg_main.cpp
	code/cgame/cg_marks.cpp
	code/cgame/cg_players.cpp
	code/cgame/cg_playerstate.cpp
	code/cgame/cg_predict.cpp
	code/cgame/cg_scoreboard.cpp
	code/cgame/cg_servercmds.cpp
	code/cgame/cg_snapshot.cpp
	code/cgame/cg_syscalls.cpp
	code/cgame/cg_text.cpp
	code/cgame/cg_view.cpp
	code/cgame/cg_weapons.cpp
	code/game/AI_Atst.cpp
	code/game/AI_Default.cpp
	code/game/AI_Droid.cpp
	code/game/AI_GalakMech.cpp
	code/game/AI_Grenadier.cpp
	code/game/AI_Howler.cpp
	code/game/AI_ImperialProbe.cpp
	code/game/AI_Interrogator.cpp
	code/game/AI_Jedi.cpp
	code/game/AI_Mark1.cpp
	code/game/AI_Mark2.cpp
	code/game/AI_MineMonster.cpp
	code/game/AI_Remote.cpp
	code/game/AI_Seeker.cpp
	code/game/AI_Sentry.cpp
	code/game/AI_Sniper.cpp
	code/game/AI_Stormtrooper.cpp
	code/game/AI_Utils.cpp
	code/game/G_Timer.cpp
	code/game/NPC.cpp
	code/game/NPC_behavior.cpp
	code/game/NPC_combat.cpp
	code/game/NPC_goal.cpp
	code/game/NPC_misc.cpp
	code/game/NPC_move.cpp
	code/game/NPC_reactions.cpp
	code/game/NPC_senses.cpp
	code/game/NPC_sounds.cpp
	code/game/NPC_spawn.cpp
	code/game/NPC_stats.cpp
	code/game/NPC_utils.cpp
	code/game/Q3_Interface.cpp
	code/game/Q3_Registers.cpp
	code/game/bg_misc.cpp
	code/game/bg_pangles.cpp
	code/game/bg_panimate.cpp
	code/game/bg_pmove.cpp
	code/game/bg_slidemove.cpp
	code/game/g_ICARUS.cpp
	code/game/g_active.cpp
	code/game/g_breakable.cpp
	code/game/g_camera.cpp
	code/game/g_client.cpp
	code/game/g_cmds.cpp
	code/game/g_combat.cpp
	code/game/g_functions.cpp
	code/game/g_fx.cpp
	code/game/g_headers.cpp
	code/game/g_inventory.cpp
	code/game/g_itemLoad.cpp
	code/game/g_items.cpp
	code/game/g_local.cpp
	code/game/g_main.cpp
	code/game/g_mem.cpp
	code/game/g_misc.cpp
	code/game/g_misc_model.cpp
	code/game/g_missile.cpp
	code/game/g_mover.cpp
	code/game/g_nav.cpp
	code/game/g_navigator.cpp
	code/game/g_navnew.cpp
	code/game/g_object.cpp
	code/game/g_objectives.cpp
	code/game/g_ref.cpp
	code/game/g_roff.cpp
	code/game/g_savegame.cpp
	code/game/g_session.cpp
	code/game/g_spawn.cpp
	code/game/g_svcmds.cpp
	code/game/g_target.cpp
	code/game/g_trigger.cpp
	code/game/g_turret.cpp
	code/game/g_usable.cpp
	code/game/g_utils.cpp
	code/game/g_weapon.cpp
	code/game/g_weaponLoad.cpp
	code/game/game.def
	code/game/genericparser2.cpp
	code/game/q_math.cpp
	code/game/q_shared.cpp
	code/game/wp_saber.cpp
	code/icarus/BlockStream.cpp
	code/icarus/Instance.cpp
	code/icarus/Sequence.cpp
	code/icarus/Sequencer.cpp
	code/icarus/TaskManager.cpp
	code/qcommon/tri_coll_test.cpp
	code/ui/gameinfo.cpp
)

add_library(jk2gamex86 SHARED ${GAME_SOURCE_FILES})
set_target_properties(jk2gamex86 PROPERTIES PREFIX "")

set(STARWARS_SOURCE_FILES
	code/client/cl_cgame.cpp
	code/client/cl_cin.cpp
	code/client/cl_console.cpp
	code/client/cl_input.cpp
	code/client/cl_keys.cpp
	code/client/cl_main.cpp
	code/client/cl_mp3.cpp
	code/client/cl_parse.cpp
	code/client/cl_scrn.cpp
	code/client/cl_ui.cpp
	code/client/snd_ambient.cpp
	code/client/snd_dma.cpp
	code/client/snd_mem.cpp
	code/client/snd_mix.cpp
	code/client/snd_music.cpp
	code/client/vmachine.cpp
	code/encryption/buffer.cpp
	code/encryption/cpp_interface.cpp
	code/encryption/sockets.cpp
	code/game/genericparser2.cpp
	code/game/q_math.cpp
	code/game/q_shared.cpp
	code/ghoul2/G2_API.cpp
	code/ghoul2/G2_bolts.cpp
	code/ghoul2/G2_bones.cpp
	code/ghoul2/G2_misc.cpp
	code/ghoul2/G2_surfaces.cpp
	code/jpeg-6/jcapimin.cpp
	code/jpeg-6/jccoefct.cpp
	code/jpeg-6/jccolor.cpp
	code/jpeg-6/jcdctmgr.cpp
	code/jpeg-6/jchuff.cpp
	code/jpeg-6/jcinit.cpp
	code/jpeg-6/jcmainct.cpp
	code/jpeg-6/jcmarker.cpp
	code/jpeg-6/jcmaster.cpp
	code/jpeg-6/jcomapi.cpp
	code/jpeg-6/jcparam.cpp
	code/jpeg-6/jcphuff.cpp
	code/jpeg-6/jcprepct.cpp
	code/jpeg-6/jcsample.cpp
	code/jpeg-6/jctrans.cpp
	code/jpeg-6/jdapimin.cpp
	code/jpeg-6/jdapistd.cpp
	code/jpeg-6/jdatadst.cpp
	code/jpeg-6/jdatasrc.cpp
	code/jpeg-6/jdcoefct.cpp
	code/jpeg-6/jdcolor.cpp
	code/jpeg-6/jddctmgr.cpp
	code/jpeg-6/jdhuff.cpp
	code/jpeg-6/jdinput.cpp
	code/jpeg-6/jdmainct.cpp
	code/jpeg-6/jdmarker.cpp
	code/jpeg-6/jdmaster.cpp
	code/jpeg-6/jdpostct.cpp
	code/jpeg-6/jdsample.cpp
	code/jpeg-6/jdtrans.cpp
	code/jpeg-6/jerror.cpp
	code/jpeg-6/jfdctflt.cpp
	code/jpeg-6/jidctflt.cpp
	code/jpeg-6/jmemmgr.cpp
	code/jpeg-6/jmemnobs.cpp
	code/jpeg-6/jutils.cpp
	code/linux/linux_console.cpp
	code/linux/linux_glimp.cpp
	code/linux/linux_main.cpp
	code/linux/linux_qgl.cpp
	code/linux/linux_snddma.cpp
	code/mp3code/cdct.c
	code/mp3code/csbt.c
	code/mp3code/csbtL3.c
	code/mp3code/csbtb.c
	code/mp3code/cup.c
	code/mp3code/cupL1.c
	code/mp3code/cupini.c
	code/mp3code/cupl3.c
	code/mp3code/cwin.c
	code/mp3code/cwinb.c
	code/mp3code/cwinm.c
	code/mp3code/hwin.c
	code/mp3code/l3dq.c
	code/mp3code/l3init.c
	code/mp3code/mdct.c
	code/mp3code/mhead.c
	code/mp3code/msis.c
	code/mp3code/towave.c
	code/mp3code/uph.c
	code/mp3code/upsf.c
	code/mp3code/wavep.c
	code/qcommon/cm_load.cpp
	code/qcommon/cm_patch.cpp
	code/qcommon/cm_polylib.cpp
	code/qcommon/cm_test.cpp
	code/qcommon/cm_trace.cpp
	code/qcommon/cmd.cpp
	code/qcommon/common.cpp
	code/qcommon/cvar.cpp
	code/qcommon/files.cpp
	code/qcommon/hstring.cpp
	code/qcommon/md4.cpp
	code/qcommon/msg.cpp
	code/qcommon/net_chan.cpp
	code/qcommon/strip.cpp
	code/qcommon/unzip.cpp
	code/renderer/matcomp.c
	code/renderer/tr_animation.cpp
	code/renderer/tr_backend.cpp
	code/renderer/tr_bsp.cpp
	code/renderer/tr_cmds.cpp
	code/renderer/tr_curve.cpp
	code/renderer/tr_draw.cpp
	code/renderer/tr_font.cpp
	code/renderer/tr_ghoul2.cpp
	code/renderer/tr_image.cpp
	code/renderer/tr_init.cpp
	code/renderer/tr_jpeg_interface.cpp
	code/renderer/tr_light.cpp
	code/renderer/tr_main.cpp
	code/renderer/tr_marks.cpp
	code/renderer/tr_mesh.cpp
	code/renderer/tr_model.cpp
	code/renderer/tr_noise.cpp
	code/renderer/tr_quicksprite.cpp
	code/renderer/tr_scene.cpp
	code/renderer/tr_shade.cpp
	code/renderer/tr_shade_calc.cpp
	code/renderer/tr_shader.cpp
	code/renderer/tr_shadows.cpp
	code/renderer/tr_sky.cpp
	code/renderer/tr_stl.cpp
	code/renderer/tr_surface.cpp
	code/renderer/tr_surfacesprites.cpp
	code/renderer/tr_world.cpp
	code/renderer/tr_worldeffects.cpp
	code/server/exe_headers.cpp
	code/server/sv_ccmds.cpp
	code/server/sv_client.cpp
	code/server/sv_game.cpp
	code/server/sv_init.cpp
	code/server/sv_main.cpp
	code/server/sv_savegame.cpp
	code/server/sv_snapshot.cpp
	code/server/sv_world.cpp
	code/ui/ui_atoms.cpp
	code/ui/ui_connect.cpp
	code/ui/ui_debug.cpp
	code/ui/ui_main.cpp
	code/ui/ui_shared.cpp
	code/ui/ui_syscalls.cpp
)

set (EXTRA_LIBS ${EXTRA_LIBS} dl)
set (EXTRA_LIBS ${EXTRA_LIBS} openal)
set (EXTRA_LIBS ${EXTRA_LIBS} GL)
set (EXTRA_LIBS ${EXTRA_LIBS} X11)
set (EXTRA_LIBS ${EXTRA_LIBS} Xxf86vm)
set (EXTRA_LIBS ${EXTRA_LIBS} Xxf86dga)
set (EXTRA_LIBS ${EXTRA_LIBS} Xrandr)
set (EXTRA_LIBS ${EXTRA_LIBS} rt)
set (EXTRA_LIBS ${EXTRA_LIBS} pthread)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-m32")

add_executable(jk2sp ${STARWARS_SOURCE_FILES})
target_link_libraries (jk2sp  ${EXTRA_LIBS})
set_target_properties (jk2sp PROPERTIES COMPILE_DEFINITIONS "_JK2EXE;_M_IX86")
