STANDALONE = true

CPUS["I86"] = true

function standalone()
    files{
		PROJECT_DIR .. "src/dosplay/main.cpp",
		PROJECT_DIR .. "src/dosplay/dosplay.cpp",
    }
end

