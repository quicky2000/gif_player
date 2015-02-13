/*    This file is part of gif_player
      Copyright (C) 2015  Julien Thevenon ( julien_thevenon at yahoo.fr )

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "parameter_manager.h"
#include "gif.h"
#include "gif_graphic_block.h"
#include "gif_color_table.h"
#include "gif_application_extension.h"
#include "gif_graphic_control_extension.h"
#include "gif_plain_text_extension.h"
#include "simple_gui.h"
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>

#include <signal.h>
#ifdef _WIN32
#include <windows.h>
#endif

bool g_stop = false;

//------------------------------------------------------------------------------
void sig_handler(int p_sig)
{
  std::cout << "===================> Receive Control-C : execution will stop" << std::endl ;
  g_stop = true;
}

int main(int argc,char ** argv)
{


  try
    {
      // Defining application command line parameters
      parameter_manager::parameter_manager l_param_manager("gif_player.exe","--",1);
      parameter_manager::parameter_if l_file_name_parameter("file_name",false);
      l_param_manager.add(l_file_name_parameter);
  
      // Treating parameters
      l_param_manager.treat_parameters(argc,argv);

      std::string l_file_name = l_file_name_parameter.get_value<std::string>();
      std::ifstream l_file;
      l_file.open(l_file_name.c_str(),std::ifstream::binary);
      if(!l_file) throw quicky_exception::quicky_runtime_exception("Unable to read file \""+l_file_name+"\"",__LINE__,__FILE__);
     

      lib_gif::gif l_gif(l_file);
      
      l_file.close();

      simple_gui l_gui;
      unsigned int l_gif_width = l_gif.get_width();
      unsigned int l_gif_height = l_gif.get_height();
      l_gui.createWindow(l_gif_width,l_gif_height);


      lib_gif::gif_color_table const *l_color_table = nullptr;
      if(l_gif.get_global_color_table_flag())
        {
          l_color_table = & l_gif.get_global_color_table();
          lib_gif::gif_color l_color = (*l_color_table)[l_gif.get_background_index()];
          uint32_t l_sdl_color = l_gui.getColorCode(l_color.get_red(),l_color.get_green(),l_color.get_blue());
          for(unsigned int l_y = 0 ; l_y < l_gif_height ; ++l_y)
            {
              for(unsigned int l_x = 0 ; l_x < l_gif_width ; ++l_x)
                {
                  l_gui.setPixel(l_x,l_y,l_sdl_color);
                }
            }
        }
      
      int l_loop_counter = 0;
      const lib_gif::gif_graphic_control_extension * l_control_extension = nullptr;

      bool l_ctrl_c_handler = false;
      do
	{
	  for(unsigned int l_index = 0 ; l_index < l_gif.get_nb_data_block() && !g_stop; ++l_index)
	    {
	      const lib_gif::gif_data_block & l_data_block = l_gif.get_data_block(l_index);
	      switch(l_data_block.get_type())
		{
                case lib_gif::gif_data_block::t_gif_data_block_type::GRAPHICAL_CONTROL_EXTENSION :
                  l_control_extension = static_cast<const lib_gif::gif_graphic_control_extension*>(&l_data_block);
                  if(!l_ctrl_c_handler)
                    {
                      std::cout << "Create CTRL+C handler" << std::endl ;
                      l_ctrl_c_handler = true;
#ifndef _WIN32
                      //Preparing signal handling to manage stop
                      // Struct for signal manager
                      struct sigaction l_signal_action;
                                
                      l_signal_action.sa_handler=sig_handler;
                      // Reset flags
                      l_signal_action.sa_flags=0;
                      // No bblock of specific signals
                      sigemptyset(&l_signal_action.sa_mask);
                            
                      // Setting up manager
                      sigaction(SIGINT,&l_signal_action,0);
#else
                      signal(SIGINT,sig_handler);
#endif
                    }

                  break;
		case lib_gif::gif_data_block::t_gif_data_block_type::COMMENT_EXTENSION :
                  std::cout << l_data_block;
		  break;
		case lib_gif::gif_data_block::t_gif_data_block_type::APPLICATION_EXTENSION :
		  {
		    const lib_gif::gif_application_extension * l_application_extension = static_cast<const lib_gif::gif_application_extension*>(&l_data_block);
		    if(l_application_extension->is_supported() && !l_loop_counter)
		      {
			l_loop_counter = l_application_extension->get_loop_counter();
			if(!l_loop_counter)
			  {
			    l_loop_counter = -1;
			  }
		      }
		  }
		  break;
		case lib_gif::gif_data_block::t_gif_data_block_type::GRAPHIC_BLOCK:
		  {
		    const lib_gif::gif_graphic_block & l_graphic_block = * static_cast<const lib_gif::gif_graphic_block*>(&l_data_block);
		    const unsigned int l_left_position = l_graphic_block.get_left_position();
		    const unsigned int l_top_position = l_graphic_block.get_top_position();
		    const unsigned int l_width = l_graphic_block.get_width();
		    const unsigned int l_height = l_graphic_block.get_height();
                
		    std::string l_error;
		    if(l_width + l_left_position > l_gif_width)
		      {
			std::stringstream l_max_x_stream;
			l_max_x_stream << l_width + l_left_position;
			std::stringstream l_gif_width_stream;
			l_gif_width_stream << l_gif_width;
			l_error = "Max x coordinate "+l_max_x_stream.str()+" is greater than width "+l_gif_width_stream.str();
		      }
		    if(l_height + l_top_position > l_gif_height)
		      {
			std::stringstream l_max_y_stream;
			l_max_y_stream << l_height + l_top_position;
			std::stringstream l_gif_height_stream;
			l_gif_height_stream << l_gif_height;
			l_error = "Max y coordinate "+l_max_y_stream.str()+" is greater than height "+l_gif_height_stream.str();
		      }
		    if("" != l_error) throw quicky_exception::quicky_logic_exception(l_error,__LINE__,__FILE__);

		    void *l_saved_rectangle = nullptr;
		    if(l_control_extension && 3 == l_control_extension->get_disposal_method())
		      {
			l_saved_rectangle = l_gui.export_rectangle(l_left_position,l_top_position,l_width,l_height);
		      }

		    if(l_graphic_block.is_image())
		      {
			const lib_gif::gif_image & l_image = l_graphic_block.get_image();
			lib_gif::gif_color_table const *l_saved_color_table = l_color_table;
			if(l_image.get_local_color_table_flag())
			  {
			    l_color_table = & l_image.get_local_color_table();
			  }
			if(!l_color_table) throw quicky_exception::quicky_logic_exception("No colour table available",__LINE__,__FILE__);

			for(unsigned int l_y = 0 ; l_y < l_height ; ++l_y)
			  {
                            unsigned int l_computed_y = !l_image.get_interlace_flag() ? l_y : l_image.deinterlace(l_y);
			    for(unsigned int l_x = 0 ; l_x < l_width ; ++l_x)
			      {
				if(!l_control_extension || !l_control_extension->get_transparent_color_flag() || l_control_extension->get_transparent_color_index() != l_image.get_color_index(l_x,l_y))
				  {
				    lib_gif::gif_color l_color = (*l_color_table)[l_image.get_color_index(l_x,l_computed_y)];
				    uint32_t l_sdl_color = l_gui.getColorCode(l_color.get_red(),l_color.get_green(),l_color.get_blue());
				    l_gui.setPixel(l_left_position + l_x,l_top_position + l_y,l_sdl_color);
				  }
			      }
			  }
			l_gui.refresh();
			l_color_table = l_saved_color_table;
		      }
		    if(l_control_extension)
                      {

                        if(l_control_extension->get_delay_time())
                          {
                            unsigned int l_delay = 10000 * l_control_extension->get_delay_time();
                            if(l_control_extension->get_user_input_flag())
                              {
                                fd_set l_read_fd_set;
                                struct timeval l_timeval;
                                int l_ret_val = 0;
                                /* Watch stdin (fd 0) to see when it has input. */
                                FD_ZERO(&l_read_fd_set);
                                FD_SET(0, &l_read_fd_set);
                            
                                /* Wait up to five seconds. */
                                l_timeval.tv_sec = 0;
                                l_timeval.tv_usec = l_delay;

                                l_ret_val = select(1, &l_read_fd_set, NULL, NULL, &l_timeval);

                                if (l_ret_val == -1)
                                  {
                                    perror("select()");
                                  }

                              }
                            else
                              {
                                usleep(l_delay);
                              }
                          }
                        switch(l_control_extension->get_disposal_method())
                          {
                          case 0:
                          case 1:
                            break;
                          case 2:
                            if(l_gif.get_global_color_table_flag())
                              {
                                l_color_table = & l_gif.get_global_color_table();
                                lib_gif::gif_color l_color = (*l_color_table)[l_gif.get_background_index()];
                                uint32_t l_sdl_color = l_gui.getColorCode(l_color.get_red(),l_color.get_green(),l_color.get_blue());
                                l_gui.set_rectangle(l_left_position,l_top_position,l_width,l_height,l_sdl_color);
                              }
                            break;
                          case 3:
			    {
			      l_gui.import_rectangle(l_left_position,l_top_position,l_width,l_height,l_saved_rectangle);
			      l_gui.free_rectangle(l_saved_rectangle);
			      l_saved_rectangle = nullptr;
			    }
                            break;
                          default:
                            std::cout << "Unsupported disposal method : " << l_control_extension->get_disposal_method() << std::endl ;
                          }
		      }
		    l_control_extension = nullptr;
		  }
		  break;
		case lib_gif::gif_data_block::t_gif_data_block_type::TRAILER:
		  {
		  }
		  break;
		default:
		  throw quicky_exception::quicky_logic_exception("Unsupported gif_data_block type "+lib_gif::gif_data_block::type_to_string(l_data_block.get_type()),__LINE__,__FILE__);
		}
	    }
	  if(l_loop_counter > 0)
	    {
	      --l_loop_counter;
	    }
	}while(l_loop_counter && !g_stop);
      if(!g_stop)
	{
	  std::cout << "Type any key" << std::endl ;
	  getchar();
	}

    }
  catch(quicky_exception::quicky_runtime_exception & e)
    {
      std::cout << "ERROR : " << e.what() << std::endl ;
      return(-1);
    }
  catch(quicky_exception::quicky_logic_exception & e)
    {
      std::cout << "ERROR : " << e.what() << std::endl ;
      return(-1);
    }
  return 0;
}
//EOF
