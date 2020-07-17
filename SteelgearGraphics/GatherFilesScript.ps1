$libs = "SteelgearGraphics64.lib", "SteelgearGraphics64_D.lib", "SteelgearGraphics32.lib", "SteelgearGraphics32_D.lib"

Get-ChildItem -Path "Distribute" -Include *.* -File -Recurse | foreach { $_.Delete()}

foreach ($file in $libs)
{
	

	$path = Get-ChildItem -Path "" -Filter $file -Recurse -ErrorAction SilentlyContinue -Force
	if ($path)
	{
		Copy-Item $path.FullName -Destination "Distribute\\Libraries\\"
	}
	else
	{
		Write-Host "Could not find file:" $file
	}
}

Get-ChildItem -Path "SteelgearGraphics" -Include *.h* -File -Recurse | foreach { Copy-Item $_.FullName -Destination "Distribute\\Headers\\"}