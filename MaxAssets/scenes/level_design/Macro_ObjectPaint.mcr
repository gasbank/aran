-------------------------------------------------------------------------------
-- ObjectPaint.ms
-- Paints using current selection as the brush.
-- By Simon Feltman [Kinetix]  simon.feltman@ktx.com
-- TODO: have option to align object with one another

-- Edited by Frank D, for random painting 
-------------------------------------------------------------------------------
MacroScript ObjectPainter
	category:"MAX Script Utilities"
	tooltip:"Object Painter"
	buttontext:"Object Painter"
(
	global ObjectPaintFloater
	rollout ObjectPaint "Object Paint"
	(
		spinner spn_spacing "Spacing:" range:[0, 99999, 20] align:#center
		checkbutton btn_paint "Paint" width:116
	
		tool ObjectPaint_tool prompt:"Paint using selected object as brush"
		(
			local lastObject
	
			on mousePoint clickno do
			(
				if selection[1] != undefined and clickno == 1 then
				(
					lastObject = copy selection[Random 1 selection.count] pos:worldPoint -- Added Frank D
				)
				else
				(
					btn_paint.state = off
					#stop
				)
			)
	
			on mouseMove clickno do
			(
				if clickno == 2 then
				(
					local vec = worldPoint - lastObject.pos
					local distSquared = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z
					if distSquared > (spn_spacing.value*spn_spacing.value) then
					(
						vec = Normalize vec
						lastObject = reference selection[Random 1 selection.count] pos:(vec * spn_spacing.value + lastObject.pos)
					)
				)
			)
	
			on mouseAbort clickno do
				btn_paint.state = off
		)
	
		on btn_paint changed state do
			if state == on then StartTool ObjectPaint_tool
			else StopTool ObjectPaint_tool
	)
ObjectPaintFloater = newRolloutFloater "Object Painter" 200 120
addRollout ObjectPaint ObjectPaintFloater
)