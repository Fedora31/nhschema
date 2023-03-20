# nhschema

See: [no-hats-bgum](https://github.com/Fedora31/no-hats-bgum),
[nhupdater2](https://github.com/Fedora31/nhupdater2)

nhschema is a program capable to read (most of) the cosmetic
items from TF2's client item schema and outputing them in a format
readable by [nhupdater2](https://github.com/Fedora31/nhupdater2).
It was created to allow the generation of new versions of
[no-hats-bgum](https://github.com/Fedora31/no-hats-bgum) much
more quickly, as reading the item schema allows to more
reliably determine which cosmetic items are replacing bodygroups.

The parser isn't able to handle every cosmetic present in the
schema as some of them possess some unique quirks that make it
difficult to create a uniform way to parse for information.
Nevertheless, it should be able to handle recent cosmetics
relatively well.

## Limitations

The passed schema must be the one used by Team Fortress 2, as
some entry name and their location are hard-coded into the
program.

Some cosmetic items do not possess common entry names (e.g.
"The Grandmaster"). Their paths are thus not handled correctly.

Cosmetic items which possess styles that hide/show a
bodygroup will have the bodygroup reappear for both styles,
wether or not it wasn't disabled to begin with (resulting to
overlapping hats, which is in most circumstances not noticeable
unless you modify the mod with
[nhcustom2](https://github.com/Fedora31/nhcustom2)). For hats
that use the same model between styles (e.g. "Honest Halo"),
this limitation is with how no-hats-bgum works. for the others,
it is a limitation of this program.

## Usage

`./nhschema <items_game.txt`

`items_game.txt` is the item schema which can be found at
`tf/scripts/items/items_game.txt`.
