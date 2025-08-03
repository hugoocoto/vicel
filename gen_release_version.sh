output_file="version.txt"

get_season() {
    month=$(date +%m)
    case $month in
        12|01|02) echo "winter" ;;
        03|04|05) echo "spring" ;;
        06|07|08) echo "summer" ;;
        09|10|11) echo "autumn" ;;
    esac
}

month_abbr=$(date +%b | tr '[:upper:]' '[:lower:]')
season=$(get_season)

if [[ -f "$output_file" && -s "$output_file" ]]; then
    last_line=$(tail -n 1 "$output_file")
    last_number=$(echo "$last_line" | grep -o '[0-9]\{6\}$')
    new_number=$(printf "%06d" $((10#$last_number + 1)))
else
    new_number="000001"
fi

new_line="vicel_unstable_release_${season}_${month_abbr}_${new_number}"

echo "$new_line" > "$output_file"

