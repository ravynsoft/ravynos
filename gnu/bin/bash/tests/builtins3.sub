# declare -g added in bash-4.2
f=4

foo()
{
	declare -g f=8
	declare -g bar=4

	echo inside
}

echo before: f = $f
foo
echo after: f = $f bar = $bar
