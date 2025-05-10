return {
    name = "std",
    validate = function(data, row, col, cell)
        for i = 1, 9 do
            if grid:get(row, i) == cell or grid:get(i, col) == cell then
                return false
            end
        end

        local start_row = row - (row - 1) % 3
        local start_col = col - (col - 1) % 3

        for i = 0, 2 do
            for j = 0, 2 do
                if grid:get(start_row + i, start_col + j) == cell then
                    return false
                end
            end
        end

        return true
    end
}
