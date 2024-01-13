# Performance and quality analysis

Each stable release (and some betas) of todds includes a performance and quality analysis comparing todds with other popular texture encoders.

This document describes how this analysis is performed, allowing to replicate the results, and test on different hardware.

These tests are always performed on Windows because the [Texconv](https://github.com/Microsoft/DirectXTex/wiki/Texconv) tool is only available for that operative system.

## Required software

### Encoders

All encoders are set to use the BC7 format and to create mipmaps.

* todds: Compiled following these steps: https://github.com/todds-encoder/todds/wiki/Static-builds-on-Windows. Uses Lanczos interpolation to calculate mipmaps.
* [Texconv](https://github.com/Microsoft/DirectXTex/wiki/Texconv): Uses WIC to create mipmaps.

### Tools

* [ImageMagick](https://imagemagick.org/index.php): image metrics.

### Optional encoders

* [bc7enc_rdo](https://github.com/richgel999/bc7enc_rdo): todds encodes DDS files using the libraries implemented for this tool. Checking bc7enc_rdo can identify potential performance or quality regressions. bc7enc_rdo lacks mipmap support, and therefore is performing less calculations and producing smaller DDS files than the other tools.

## Datasets

The analysis is performed against texture sets available under free licenses. The datasets have been chosen to represent real use cases along with some extreme cases. This section describes each dataset and includes Powershell instructions to set them up.

The flatten tool is included in the tools subfolder of this repository. It is used to flatten the subfolders of a dataset to copy all textures into a single folder, as some of the other texture encoders require this step.

### [Crawl Stone Soup](https://github.com/crawl/tiles)

This dataset tests encoding a large number of small textures, with a size of 32x32.

```
git clone https://github.com/crawl/tiles.git
python .\flatten.py [Path to checkout]\tiles\releases\Nov-2015 [Path to datasets]\00_crawl_stone_soup
```

### [Biomes! Prehistoric](https://steamcommunity.com/sharedfiles/filedetails/?id=2860715703)

This dataset contains many textures with sizes between 1024x1024 and 128x128.

```
git clone https://github.com/biomes-team/BiomesPrehistoric.git
python .\flatten.py [Path to checkout]\BiomesPrehistoric\Textures\ [Path to datasets]\01_biomes_prehistoric
```

### [AmbientCG 2K](https://ambientcg.com/view?id=Tiles101)

This dataset contains multiple 2K textures, including normal, displacement and roughness.

```
Invoke-WebRequest https://ambientcg.com/get?file=Tiles101_2K-PNG.zip -OutFile Tiles101_2K.zip
Expand-Archive .\Tiles101_2K.zip -DestinationPath 02_ambientCG_2K
Invoke-WebRequest https://ambientcg.com/get?file=Ground054_2K-PNG.zip -OutFile Ground054_2K.zip
Expand-Archive .\Ground054_2K.zip -DestinationPath 02_ambientCG_2K
Invoke-WebRequest https://ambientcg.com/get?file=MetalPlates006_2K-PNG.zip -OutFile MetalPlates006_2K.zip
Expand-Archive .\MetalPlates006_2K.zip -DestinationPath 02_ambientCG_2K
rm .\02_ambientCG_2K\*.usd*
rm .\02_ambientCG_2K\*.jpg
```

### [Metal Plates 014 4K](https://ambientcg.com/view?id=MetalPlates014)

This dataset contains six 4K textures, including normal, displacement and roughness. Some of these textures produce DDS files with high PSNR values, useful for quality analysis.

```
Invoke-WebRequest https://ambientcg.com/get?file=MetalPlates014_4K-PNG.zip -OutFile 03_MetalPlates014_4K.zip
Expand-Archive .\03_MetalPlates014_4K.zip -DestinationPath 03_MetalPlates014_4K
rm .\03_MetalPlates014_4K\*.usd*
rm .\03_MetalPlates014_4K\MetalPlates014_PREVIEW.jpg
```

### [Bricks 075 A 8K](https://ambientcg.com/view?id=Bricks075A)

This dataset contains six 8K textures, including normal, displacement and roughness.

```
Invoke-WebRequest https://ambientcg.com/get?file=Bricks075A_8K-PNG.zip -OutFile 04_Bricks075A_8K.zip
Expand-Archive .\04_Bricks075A_8K.zip -DestinationPath 04_Bricks075A_8K
rm .\04_Bricks075A_8K\*.usd*
rm .\04_Bricks075A_8K\Bricks075A_PREVIEW.jpg
```

### [Webb Space Telescope - “Cosmic Cliffs” in the Carina Nebula](https://webbtelescope.org/contents/media/images/2022/031/01G77PKB8NKR7S8Z6HBXMYATGJ)

This dataset contains a single, huge image. todds performs better when processing multiple files of small or medium size at once. This dataset checks how much the performance of the encoder suffers when encountering the opposite case.

```
mkdir 05_cosmic_cliffs
Invoke-WebRequest https://stsci-opo.org/STScI-01GA6KKWG229B16K4Q38CH3BXS.png -OutFile 05_cosmic_cliffs\cosmic_clifs.png
```

## Analysis

After the datasets have been obtained, the analysis can be performed by using the comparedds.py tool contained in this repository.

### System information

Generates a CSV file containing hardware and software information.

```
python.exe .\comparedds.py --todds --texconv --info > [Path to results]\info.csv
```

### Dataset information

The following commands should be executed once for each dataset.

```
python.exe .\comparedds.py --todds --texconv --batch [Path to datasets]\[Dataset] [Path to output files] > [Path to results]\[dataset_name]_batch.csv
python.exe .\comparedds.py --todds --metrics [Path to datasets]\[Dataset] [Path to output files] > [Path to results]\[dataset_name]_todds_metrics.csv
python.exe .\comparedds.py --texconv --metrics [Path to datasets]\[Dataset] [Path to output files] > [Path to results]\[dataset_name]_texconv_metrics.csv
```
