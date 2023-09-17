function normal		(shader, t_base, t_second, t_detail)
	shader:begin	("wmark",	"simple")
			: sorting	(1, false)
			: blend		(true,blend.destcolor,blend.srccolor)
			: aref 		(true,0)
			: zb 		(true,false)
			: fog		(false)
			: wmark		(true)
--	shader:sampler	("s_base")      :texture	(t_base)
	shader: dx10texture ("s_base", t_base)
	shader: dx10sampler ("smp_rtlinear")
	shader: dx10color_write_enable( true, true, true, false)
end
