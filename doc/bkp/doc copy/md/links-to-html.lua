-- links-to-html.lua
function Link(el)
  el.target = string.gsub(el.target, "%.md", ".html")
  return el
end

-- https://stackoverflow.com/questions/40993488/convert-markdown-links-to-html-with-pandoc
-- 'pandoc -f markdown -t html5 types.md -o types.html --lua-filter=links-to-html.lua'