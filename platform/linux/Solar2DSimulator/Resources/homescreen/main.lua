------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------
local widget = require("widget")
local simErr, simulator = pcall(require, "simulator")
local json = require("json")
local lfs = require("lfs")
local sFormat = string.format
local recentProjectsPath = sFormat("%s/.Solar2D/Sandbox/Simulator/Documents/recents.json", os.getenv("HOME")) 
local backgroundColor = {0.14, 0.14, 0.149, 1}
local buttonBackgroundColor = {0.2, 0.2, 0.21, 1}
local mainFont = "OpenSans-Regular.ttf"
local lightFont = "OpenSans-Light.ttf"
local buttonWidth = 150
local buttonHeight = 30
local buttonFontSize = 14
local buttonYPadding = 5
local recentProjectIconSize = 30
local headerFontSize = 14
local sidePadding = 12
local recentProjectTitles = {}

display.setDefault("background", backgroundColor[1], backgroundColor[2], backgroundColor[3])

local function scriptPath()
	local str = debug.getinfo(2, "S").source:sub(2):match("(.*/)")
	return str:match("(.*/)")
 end

local function getLastPathComponent(str)
	local pathIndexes = {}

	for i = 1, #str do
		if (str:sub(i, i) == "/") then
			pathIndexes[#pathIndexes + 1] = i
		end
	end

	return string.sub(str, pathIndexes[#pathIndexes - 1], pathIndexes[#pathIndexes])
end

local function getAppName(str)
	local lastPathComponent = getLastPathComponent(str)
	return (lastPathComponent:sub(2, lastPathComponent:len() - 1))
end

local function getProjectPath(str)
	local pathIndexes = {}

	for i = 1, #str do
		if (str:sub(i, i) == "/") then
			pathIndexes[#pathIndexes + 1] = i
		end
	end

	return string.sub(str, 1, pathIndexes[#pathIndexes])
end

local function loadTable(path)
	local loc = location

	if not location then
		loc = defaultLocation
	end

	local file, errorString = io.open(path, "r")

	if not file then
		return {}
	else
		local contents = file:read("*a")
		local t = json.decode(contents)
		io.close(file)

		return t
	end
end

local function saveTable(t, path)
	local loc = location

	if not location then
		loc = defaultLocation
	end

	local file, errorString = io.open(path, "w")

	if not file then
		print("File error: " .. errorString)
		return false
	else
		file:write(json.encode(t))
		io.close(file)

		return true
	end
end

local function removeExistingProjectFromRecents(recentProjects, projectName)
	-- see if this project already exists in recent projects
	if (#recentProjects > 0) then
		local existingPosition = 0

		for i = 1, #recentProjects do
			if (recentProjects[i].formattedString == projectName) then
				existingPosition = i
				break;
			end
		end

		if (existingPosition > 0) then
			table.remove(recentProjects, existingPosition)
			saveTable(recentProjects, recentProjectsPath)
		end

		if (#recentProjects > 7) then
			for i = 7, #recentProjects do
				table.remove(recentProjects, i)
			end

			saveTable(recentProjects, recentProjectsPath)
		end
	end
end

local solar2DTuxLogo = display.newImageRect("images/logo.png", display.contentWidth / 2.5, display.contentHeight / 5)
solar2DTuxLogo.anchorX = 0
solar2DTuxLogo.anchorY = 0
solar2DTuxLogo.x = sidePadding
solar2DTuxLogo.y = 10

local latestNewsTitle = display.newText(
{
	text = "Latest News:",
	font = mainFont,
	fontSize = headerFontSize,
	align = "left"
})
latestNewsTitle.anchorX = 1
latestNewsTitle.anchorY = 0
latestNewsTitle.x = display.contentWidth - 10
latestNewsTitle.y = 20
latestNewsTitle:setFillColor(1, 1, 1, 1)

local latestNewsText = display.newText(
{
	text = "Solar2DTux is developed primarily by Danny Glover & Robert Craig. If you are in a position to support our work, please visit our homepage via the 'website' button to see our GitHub/Patreon sponsor accounts. Thank you.",
	font = lightFont,
	fontSize = headerFontSize - 1,
	width = display.contentWidth / 2.2,
	height = solar2DTuxLogo.contentHeight / 1.5,
	align = "left"
})
latestNewsText.anchorX = 1
latestNewsText.anchorY = 0
latestNewsText.x = display.contentWidth - 10
latestNewsText.y = latestNewsTitle.y + latestNewsTitle.contentHeight + 2
latestNewsText:setFillColor(1, 1, 1, 1)

-- get started
local getStartedText = display.newText(
{
	text = "Get Started",
	font = mainFont,
	fontSize = headerFontSize,
	align = "left"
})
getStartedText.anchorX = 0
getStartedText.anchorY = 0
getStartedText.x = sidePadding
getStartedText.y = solar2DTuxLogo.y + solar2DTuxLogo.contentHeight + 4
getStartedText:setFillColor(1, 1, 1, 1)

local separatorLine = display.newRect(0, 0, display.contentWidth - 20, 1)
separatorLine.anchorY = 0
separatorLine.x = display.contentCenterX
separatorLine.y = getStartedText.y + getStartedText.contentHeight + 2
separatorLine:setFillColor(unpack(buttonBackgroundColor))

local function createButton(label, onRelease)
	local button = widget.newButton(
	{
		label = label,
		emboss = false,
		shape = "roundedRect",
		width = buttonWidth,
		height = buttonHeight,
		cornerRadius = 2,
		fontSize = buttonFontSize,
		fillColor = 
		{ 
			default = buttonBackgroundColor, 
			over = {buttonBackgroundColor[1], buttonBackgroundColor[2], buttonBackgroundColor[3], 0.7}, 
		},
		labelColor = 
		{
			default = {1, 1, 1, 1},
			over = {1, 1, 1, 0.7}
		},
		onRelease = onRelease
	})

	return button
end

local cloneProjectButton = createButton("Clone A Repository", 
	function(event) 
		simulator.show("clone")
	end
)
cloneProjectButton.anchorX = 0
cloneProjectButton.x = sidePadding
cloneProjectButton.y = (separatorLine.y + separatorLine.contentHeight) + (cloneProjectButton.contentHeight * 0.5) + 4

local openProjectButton = createButton("Open Existing Project",
	function(event)
		local recentProjects = loadTable(recentProjectsPath)
		simulator.show("open")

		local projectPath = simulator.getCurrentProjectPath()

		if (projectPath == nil or projectPath:len() <= 1) then
			return
		end

		local projectName = getAppName(projectPath)
		local projectDir = projectPath

		-- see if this project already exists in recent projects
		removeExistingProjectFromRecents(recentProjects, projectName)
		table.insert(recentProjects, 1, {formattedString = projectName, fullURLString = projectDir})
		saveTable(recentProjects, recentProjectsPath)
	end
)
openProjectButton.anchorX = 0
openProjectButton.x = sidePadding
openProjectButton.y = cloneProjectButton.y + buttonHeight + buttonYPadding

local createProjectButton = createButton("Create New Project",
	function(event)
		simulator.show("new")
	end
)
createProjectButton.anchorX = 0
createProjectButton.x = sidePadding
createProjectButton.y = openProjectButton.y + buttonHeight + buttonYPadding

local openSampleCodeButton = createButton("View Sample Code",
	function(event)
		simulator.show("sampleCode")
	end
)
openSampleCodeButton.anchorX = 0
openSampleCodeButton.x = sidePadding
openSampleCodeButton.y = createProjectButton.y + buttonHeight + buttonYPadding

local reportAnIssueButton = createButton("Report An Issue",
	function(event)
		system.openURL("https://github.com/DannyGlover/Solar2DTux/issues")
	end
)
reportAnIssueButton.anchorX = 0
reportAnIssueButton.x = sidePadding
reportAnIssueButton.y = openSampleCodeButton.y + buttonHeight + buttonYPadding

local documentationButton = createButton("Documentation", 
	function(event) 
		system.openURL("https://docs.coronalabs.com/api") 
	end
)
documentationButton.anchorX = 0
documentationButton.x = sidePadding
documentationButton.y = reportAnIssueButton.y + buttonHeight + buttonYPadding

local githubButton = createButton("GitHub", 
	function(event) 
		system.openURL("https://github.com/DannyGlover/Solar2DTux") 
	end
)
githubButton.anchorX = 0
githubButton.x = sidePadding
githubButton.y = documentationButton.y + buttonHeight + buttonYPadding

local websiteButton = createButton("Website",
	function(event) 
		system.openURL("https://solar2dtux.com/") 
	end
)
websiteButton.anchorX = 0
websiteButton.x = sidePadding
websiteButton.y = githubButton.y + buttonHeight + buttonYPadding

local pluginsButton = createButton("Plugins",
	function(event) 
		system.openURL("https://plugins.solar2d.com/") 
	end
)
pluginsButton.anchorX = 0
pluginsButton.x = sidePadding
pluginsButton.y = websiteButton.y + buttonHeight + buttonYPadding

-- recent projects
local recentProjectsText = display.newText(
{
	text = "Recent Projects",
	font = mainFont,
	fontSize = headerFontSize,
	align = "left"
})
recentProjectsText.anchorX = 1
recentProjectsText.anchorY = 0
recentProjectsText.x = display.contentWidth - sidePadding
recentProjectsText.y = getStartedText.y
recentProjectsText:setFillColor(1, 1, 1, 1)

local recentProjects = loadTable(recentProjectsPath)

if (#recentProjects > 0) then
	local function openRecentProject(event)
		local details = event.target.details
		local recentProjects = loadTable(recentProjectsPath)
		removeExistingProjectFromRecents(recentProjects, details.formattedString)
		table.insert(recentProjects, 1, {formattedString = details.formattedString, fullURLString = details.fullURL})
		saveTable(recentProjects, recentProjectsPath)

		simulator.show("open", details.fullURL)
	end

	for i = 1, #recentProjects do
		local icon = nil
		local projectName = recentProjects[i].formattedString
		local projectDir = getProjectPath(recentProjects[i].fullURLString)
		projectDir = projectDir:sub(1, projectDir:len() -1)
		local projectIconFile = simulator.getPreference("welcomeScreenIconFile") or "Icon.png"
		local projectIcon = sFormat("%s/%s", projectDir, projectIconFile)

		if (lfs.attributes(projectIcon) ~= nil) then
			simulator.setProjectResourceDirectory(projectDir .. "/")
			icon = display.newImageRect(projectIconFile, system.ResourceDirectory, recentProjectIconSize, recentProjectIconSize)
		else
			icon = display.newImageRect("Icon.png", system.ResourceDirectory, recentProjectIconSize, recentProjectIconSize)
		end

		local initialYPosition = (separatorLine.y + separatorLine.contentHeight) + 4
		icon.anchorX = 1
		icon.anchorY = 0
		icon.x = display.contentWidth - sidePadding
		icon.y = i == 1 and initialYPosition or (separatorLine.y + separatorLine.contentHeight) - recentProjectIconSize - 4 + ((recentProjectIconSize + 8) * i)
		
		recentProjectTitles[#recentProjectTitles + 1] = display.newText(
		{
			text = projectName,
			font = mainFont,
			fontSize = buttonFontSize,
			align = "left"
		})
		recentProjectTitles[#recentProjectTitles].anchorX = 1
		recentProjectTitles[#recentProjectTitles].anchorY = 0
		recentProjectTitles[#recentProjectTitles].x = icon.x - icon.contentWidth - sidePadding
		recentProjectTitles[#recentProjectTitles].y = icon.y
		recentProjectTitles[#recentProjectTitles].details = {formattedString = projectName, fullURL = sFormat("%s/main.lua", projectDir)}
		recentProjectTitles[#recentProjectTitles]:addEventListener("tap", openRecentProject)

		-- limit long paths
		if (projectDir:len() > 78) then
			projectDir = sFormat("..%s", projectDir:sub(projectDir:len() - 70, projectDir:len()))
		end

		local projectPathText = display.newText(
		{
			text = projectDir,
			font = lightFont,
			fontSize = buttonFontSize - 2,
			align = "left"
		})
		projectPathText.anchorX = 1
		projectPathText.anchorY = 0
		projectPathText.x = recentProjectTitles[#recentProjectTitles].x
		projectPathText.y = recentProjectTitles[#recentProjectTitles].y + (recentProjectIconSize * 0.5)

		local separatorLine = display.newRect(0, 0, display.contentWidth / 1.70, 1)
		separatorLine.anchorX = 1
		separatorLine.anchorY = 0
		separatorLine.x = projectPathText.x
		separatorLine.y = projectPathText.y + projectPathText.contentHeight + 2
		separatorLine:setFillColor(unpack(buttonBackgroundColor))
	end
else
	local noRecentProjectsText = display.newText(
	{
		text = "No recent projects were found. Why not create one?",
		font = mainFont,
		fontSize = 18,
		align = "left"
	})
	noRecentProjectsText.anchorX = 0
	noRecentProjectsText.anchorY = 0
	noRecentProjectsText.x = getStartedText.x + getStartedText.contentWidth + 180
	noRecentProjectsText.y = recentProjectsText.y + 40
	noRecentProjectsText:setFillColor(1, 1, 1, 1)
end

local versionText = display.newText(
{
	text = ("Version: %s"):format(system.getInfo("build")),
	font = lightFont,
	fontSize = buttonFontSize,
	align = "left"
})
versionText.anchorX = 0
versionText.anchorY = 1
versionText.x = sidePadding
versionText.y = display.contentHeight - 6

local copyrightText = display.newText(
{
	text = "Copyright 2020 © Solar2D / Solar2DTux",
	font = lightFont,
	fontSize = buttonFontSize,
	align = "left"
})
copyrightText.anchorX = 1
copyrightText.anchorY = 1
copyrightText.x = display.contentWidth - sidePadding
copyrightText.y = display.contentHeight - 6

local function onResize(event)
	
end

Runtime:addEventListener("resize", onResize)

local function onMouse(event)
	local phase = event.type
	local x, y = event.x, event.y

	if (phase == "move") then
		for i = 1, #recentProjectTitles do
			local withinRange = false

			if (x >= recentProjectTitles[i].x - recentProjectTitles[i].contentWidth and x <= recentProjectTitles[i].x) then
				if (y >= recentProjectTitles[i].y and y <= recentProjectTitles[i].y + recentProjectTitles[i].contentHeight) then
					withinRange = true
				end
			end

			if (withinRange) then
				recentProjectTitles[i]:setFillColor(0.70, 0.70, 0.70)
			else
				recentProjectTitles[i]:setFillColor(1, 1, 1)
			end
		end
	end

	return true
end

Runtime:addEventListener("mouse", onMouse)
