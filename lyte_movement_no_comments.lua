do
    local total_time = 0
    local SCALE = 3
    local image_1 = lyte.load_image("assets/hero.png")
    local sprite_1_tick = 2
    local sprite_1_numticks = 5
    function lyte.tick(dt, width, height)
        total_time = total_time + dt
        if lyte.is_key_pressed("space") or lyte.is_mouse_pressed("mb1") then
            sprite_1_tick = (sprite_1_tick + 1) % sprite_1_numticks
        end
        local W = width / SCALE
        local H = height / SCALE
        local x_pos = W / 2 - 16 / 2 + math.sin(total_time) * W / 3
        local y_pos = H / 2 - 16 / 2
        lyte.cls(0.2, 0.1, 0.1, 1)
        lyte.push_matrix()
        lyte.scale(SCALE, SCALE)
        lyte.draw_image_rect(image_1, x_pos, y_pos, 16 * (sprite_1_tick), 0, 16, 16)
        lyte.pop_matrix()
        lyte.set_color(1, 1, 1, math.abs(math.sin(total_time / 2)))
        lyte.draw_text("SPACE or LMB: change sprite tick", 10, height - 40)
    end
end
