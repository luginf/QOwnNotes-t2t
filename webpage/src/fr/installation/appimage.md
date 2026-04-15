# Installer en tant qu'AppImage

Vous pouvez télécharger la dernière AppImage à partir de la [page des versions de QOwnNotes](https://github.com/pbek/QOwnNotes/releases). Elle devrait être nommée `QOwnNotes-x86_64.AppImage` sur cette page.

::: tip
If you have [jq](https://stedolan.github.io/jq/) installed, you can also download the latest AppImage directly:

```bash
# demander la version Linux la plus récente depuis l'API QOwnNotes, faire l'analyse syntaxique du JSON pour obtenir son URL et la télécharger
curl -L https://api.qownnotes.org/latest_releases/linux | jq .url | xargs curl -Lo QOwnNotes-x86_64.AppImage
```

:::

Vous pouvez ensuite modifier les autorisations d'exécution du fichier :

```bash
chmod a+x QOwnNotes-*.AppImage
```

::: tip
AppImages need `libfuse2` to run! For example under Ubuntu you can install the library with `sudo apt install libfuse2`.
:::

Afterward you should be able to execute the AppImage to run QOwnNotes.

::: warning
Si vous souhaitez utiliser la **mise à jour automatique**, assurez-vous de placer votre AppImage à un endroit où votre compte utilisateur a un accès en écriture, par exemple quelque part dans votre répertoire personnel.
:::

::: tip
Si vous rencontrez des difficultés pour exécuter l'AppImage parce que votre version de glibc est trop ancienne vous pouvez essayer l'[AppImage d'OBS](https://download.opensuse.org/repositories/home:/pbek:/QOwnNotes/AppImage/QOwnNotes-latest-x86_64.AppImage), qui devrait avoir été constuite en utilisant glibc 2.16.
:::

## Portable mode

You can run the AppImage in **portable mode** by passing the `--portable` argument:

```bash
./QOwnNotes-x86_64.AppImage --portable
```

In portable mode all application data is stored in a `Data` folder next to the AppImage file instead of in your home directory. This includes:

- Settings (`Data/QOwnNotes.ini`)
- The notes database (`Data/QOwnNotes.sqlite`)
- Your notes (`Data/Notes/`)
- Scripts, spell-check dictionaries, and other application data

This makes it easy to carry QOwnNotes together with all your notes on a USB drive or any other portable storage device.

::: tip
Make sure the directory containing the AppImage file is writable, so that the `Data` folder can be created next to it.
:::
