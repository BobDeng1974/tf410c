	#version 300 es 			
      
		
	precision mediump float;                            
	in vec2 v_tex_coord;                            
      	uniform mediump int image_proc;				
	uniform mediump int uimage_width;				
	uniform sampler2D s_luma_texture;                        
	layout (location=0) out vec4 out_color;   
	// Additive offset for BT656 YUV to RGB transform.	
	const vec3 offset = vec3(0, -0.5, -0.5);	
	// Temporary variable for YUV value	
	vec3 yuv;	
	// Temporary variable for RGB value	
	vec3 rgb;
	
	
      void main()                                         
	

      {                                                   
      
	vec4 luma_chroma;	       
	float xcoord;       
	float texture_width;       
	float texel_width;       
	vec3 yuv;		       
	vec4 res;		       
	texture_width=float(uimage_width);	
	texel_width=1.0/texture_width;
	xcoord = floor (v_tex_coord.x * texture_width);	
	luma_chroma = texture2D(s_luma_texture, v_tex_coord);	
	// just look up the brightness
	   
	yuv.y = luma_chroma.b;	
	yuv.z = luma_chroma.r;	
	   
	if (0.0 != mod(xcoord , 2.0)) 	// even
	{				
		yuv.x = (luma_chroma.a - 0.0625) * 1.1643;	
	}	
	else // odd
	{	
		yuv.x = (luma_chroma.g - 0.0625) * 1.1643;	
	}	
	  
	yuv += offset;
	
	res.r = yuv.x + 1.402 * yuv.z;	
	res.g = yuv.x - 0.3441 * yuv.y - 0.7141 * yuv.z;	
	res.b = yuv.x + 1.772 * yuv.y;	
	
	switch (image_proc)		
	{				
		case 0:	/*			
			vec4 tm1m1 = texture2D(tex,tcoord+vec2(-1,-1)*texelsize);
			vec4 tm10 = texture2D(tex,tcoord+vec2(-1,0)*texelsize);
			vec4 tm1p1 = texture2D(tex,tcoord+vec2(-1,1)*texelsize);
			vec4 tp1m1 = texture2D(tex,tcoord+vec2(1,-1)*texelsize);
			vec4 tp10 = texture2D(tex,tcoord+vec2(1,0)*texelsize);
			vec4 tp1p1 = texture2D(tex,tcoord+vec2(1,1)*texelsize);
			vec4 t0m1 = texture2D(tex,tcoord+vec2(0,-1)*texelsize);
			vec4 t0p1 = texture2D(tex,tcoord+vec2(0,-1)*texelsize);

			vec4 xdiff = -1.0*tm1m1 + -2.0*tm10 + -1.0*tm1p1 + 1.0*tp1m1 + 2.0*tp10 + 1.0*tp1p1;
			vec4 ydiff = -1.0*tm1m1 + -2.0*t0m1 + -1.0*tp1m1 + 1.0*tm1p1 + 2.0*t0p1 + 1.0*tp1p1;
			vec4 tot = sqrt(xdiff*xdiff+ydiff*ydiff);

			vec4 col = tot;*/
				break;
		
		case 2://y
			res.r = yuv.x;
			res.g = yuv.x;
			res.b = yuv.x;
		break;
		
		case 3://u
			res.r = luma_chroma.b;
			res.g = luma_chroma.b;
			res.b = luma_chroma.b;
		break;
		
		case 4://v
			res.r = luma_chroma.r;
			res.g = luma_chroma.r;
			res.b = luma_chroma.r;
		break;
		
		case 5://r
			res.g=0.0f;
			res.b=0.0f;
		break;
		case 6://g
			res.r=0.0f;
			res.b=0.0f;
		break;
		case 7://b
			res.r=0.0f;
			res.g=0.0f;
		break;
	}				
	res.a = 1.0;				
	out_color = clamp(res,vec4(0),vec4(1));
	
      }                                                   

 