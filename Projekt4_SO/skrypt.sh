function menu
{
select o in 'Polecenia rm ./temp/*' 'Polecenia cat we* | wc -c' 'Polecenia cat wy* | wc -c' 'Exit'
do
        case $o in
        "Polecenia rm ./temp/*") rm ./temp/*;;
        "Polecenia cat we* | wc -c") cat ./temp/we* | wc -c;;
        "Polecenia cat wy* | wc -c") cat ./temp/wy* | wc -c;;
        "Exit") exit 0;;
        *) echo "error";;
        esac
        menu
done
}

PS3='Twoj wybor:'
export PS3
menu
exit 0
