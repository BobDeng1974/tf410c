	//#version 300 es 			
      
	precision mediump float;                            
	varying vec2 v_tex_coord;                            
      	uniform mediump int image_proc;				
	uniform mediump int uimage_width;				
	uniform sampler2D s_luma_texture;                        
	vec4 res;
	vec4 rgb;
      void main()                                         
	

      {
	rgb= texture2D(s_luma_texture, v_tex_coord);
	res=rgb;
	gl_FragColor = clamp(res,vec4(0),vec4(1));
	
      }                                                   

 