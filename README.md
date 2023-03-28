# nhschema

See: [no-hats-bgum](https://github.com/Fedora31/no-hats-bgum),
[nhupdater2](https://github.com/Fedora31/nhupdater2),
[nhcustom2](https://github.com/Fedora31/nhcustom2)

nhschema is a program capable to read (most of) the cosmetic
items from TF2's client item schema and outputing them in a format
readable by [nhupdater2](https://github.com/Fedora31/nhupdater2).
It was created to allow the generation of new versions of
[no-hats-bgum](https://github.com/Fedora31/no-hats-bgum) much
more quickly, as reading the item schema allows to more
reliably determine which cosmetic items are replacing bodygroups.

The parser hasn't been thoroughly tested against all possible
edge cases that may exist in the item schema. Nevertheless, it
should work well with recent cosmetics.

## Limitations

The passed schema must be the one used by Team Fortress 2, as
some entry names and their location are hard-coded into the
program.

For hats that use the same model between styles (e.g. "Dead of Night"),
bodygroups will reappear for all of those styles, wether or not they were
disabled to begin with (resulting in overlapping hats, which is in
most circumstances not noticeable unless you modify the mod with
[nhcustom2](https://github.com/Fedora31/nhcustom2)). This isn't
really an issue with this program but with how no-hats-bgum works,
which can't toggle bodygroups on and off with the same model.

Hats that use the same model between classes (e.g. "Honest Halo")
aren't supported and are treated as hats that don't hide any bodygroups.

A lot of entries (mostly medals) use the same model, this results
in the program outputting a lot of duplicated lines. To speed up
nhupdater2, you may want to pipe the output of this program through
`sort -u`.

## Usage

`./nhschema <items_game.txt`

`items_game.txt` is the item schema which can be found at
`tf/scripts/items/items_game.txt`.
