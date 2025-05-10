return {
    name = "thermo",
    validate = function(data, row, col, cell)
        local current_index = nil
        for i, cell_data in ipairs(data.cells) do
            if cell_data.row == row and cell_data.col == col then
                current_index = i
                break
            end
        end

        if current_index == nil then
            return true
        end

        local prev_cells = current_index - 1
        local next_cells = #data.cells - current_index

        if cell - 1 < prev_cells or 9 - cell < next_cells then
            return false
        end

        for i = 1, current_index - 1 do
            local prev_cell = data.cells[i]
            local prev_value = grid:get(prev_cell.row, prev_cell.col)
            if prev_value ~= 0 and prev_value >= cell then
                return false
            end
        end

        for i = current_index + 1, #data.cells do
            local next_cell = data.cells[i]
            local next_value = grid:get(next_cell.row, next_cell.col)
            if next_value ~= 0 and next_value <= cell then
                return false
            end
        end

        return true
    end
}
