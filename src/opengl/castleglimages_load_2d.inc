{
  Copyright 2001-2014 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ Part of CastleGLImages unit: 2D textures sizing and loading.
  Also decompressing GPU-compressed 2D textures, since it's technically related. }

{$ifdef read_interface}

{ ----------------------------------------------------------------------
  Adjusting image sizes to load them as textures.
  Usually you don't need these functions, LoadGLTexture* and TGLImage
  and such call it automatically when needed. }

type
  TTextureSizing = (
    { Texture size does not have to be a power of two
      (unless GLFeatures.TextureNonPowerOfTwo = @false, in which case all textures
      must have power-of-two, and then tsAny may be scaled to satisfy it
      (but it still will not be scaled for GLTextureScale)).
      It is not affected by GLTextureScale. }
    tsAny,
    { Texture size must be a power of two.
      It is not affected by GLTextureScale, because we cannot scale it. }
    tsRequiredPowerOf2,
    { Texture size must be a power of two.
      It is affected by GLTextureScale, we can scale it. }
    tsScalablePowerOf2
  );

{ Resize the image to a size accepted as GL_TEXTURE_2D texture size
  for OpenGL. It tries to resize to a larger size, not smaller,
  to avoid losing image information.

  It also makes texture have power-of-two size, if Sizing <> tsAny
  (or if GLFeatures.TextureNonPowerOfTwo = @false).
  This is a must for normal textures, used for 3D rendering
  (with mipmapping and such).
  Using OpenGL non-power-of-2 textures is not good for such usage case,
  some OpenGLs crash (ATI),
  some are ultra slow (NVidia), some cause artifacts (Mesa).
  OpenGL ES explicitly limits what you can do with non-power-of-2.
  Sample model using non-power-of-2 is in inlined_textures.wrl.

  Use Sizing = tsAny only for textures that you plan to use
  for drawing GUI images by TGLImage.

  @groupBegin }
procedure ResizeForTextureSize(var r: TCastleImage; const Sizing: TTextureSizing);
function ResizeToTextureSize(const r: TCastleImage; const Sizing: TTextureSizing): TCastleImage;
function ResizeToTextureSize(const Size: Cardinal; const Sizing: TTextureSizing): Cardinal;
{ @groupEnd }

{ Does image have proper size for 2D OpenGL texture.
  See ResizeForTextureSize. Note that this checks glGet(GL_MAX_TEXTURE_SIZE),
  so requires initialized OpenGL context. }
function IsTextureSized(const r: TEncodedImage; const Sizing: TTextureSizing): boolean;

function IsTextureSized(const Width, Height: Cardinal; const Sizing: TTextureSizing): boolean;

var
  { Scaling for all textures loaded to OpenGL.
    This allows you to conserve GPU memory.
    Each size (width, height, and (for 3D textures) depth) is scaled
    by 1 / 2^GLTextureScale.
    So value = 1 means no scaling, value = 2 means that each size is 1/2
    (texture area is 1/4), value = 3 means that each size is 1/4 and so on.

    Note that textures used for GUI, by TGLImage
    (more precisely: all non-power-of-2 textures) avoid this scaling entirely. }
  GLTextureScale: Cardinal = 1;

  { Contraints the scaling done by GLTextureScale.
    Scaling caused by GLTextureScale cannot scale texture to something
    less than GLTextureMinSize. If texture size was already < GLTextureMinSize,
    it is not scaled at all by GLTextureScale.
    This must be a power of two. }
  GLTextureMinSize: Cardinal = 16;

{ Load new texture to OpenGL. Generates new texture number by glGenTextures,
  then binds this texture, and loads it's data.

  Takes care of UNPACK_ALIGNMENT inside (if needed, we'll change it and
  later revert back, so that the texture is correctly loaded).

  Sets texture minification, magnification filters and wrap parameters.

  Changes currently bound texture to this one (returned).

  If mipmaps will be needed (this is decided looking at Filter.Minification)
  we will load them too.

  @orderedList(
    @item(
      As a first try, if DDSForMipmaps is non-nil
      and has mipmaps (DDSForMipmaps.Mipmaps), we will load these mipmaps.
      DDSForMipmaps must be a normal 2D texture (DDSType = dtTexture).

      Otherwise, we'll try to generate mipmaps, using various OpenGL mechanisms.)

    @item(
      We will try using GenerateMipmap functionality to generate mipmaps on GPU.
      If not available, for uncompressed textures, we will generate mipmaps on CPU.
      For compressed textures, we will change minification filter to simple
      minLinear and make OnWarning.)
  )

  @param(GUITexture If @true, the texture is not forced to have sizes
    as power-of-two. For TTextureSizing, it is like tsAny instead
    of tsScalablePowerOf2.
    Also, mipmaps are always disabled, regardless of requested
    Filter value, since mipmaps cannot work without power-of-two.)

  @raises(ETextureLoadError If texture cannot be loaded for whatever reason.
    This includes ECannotLoadS3TCTexture if the S3TC texture cannot be
    loaded for whatever reason.
    This includes EInvalidImageForOpenGLTexture if Image class is invalid
    for an OpenGL texture.)

  @raises(EImageClassNotSupportedForOpenGL When Image class is not supported
    by OpenGL.)

  @groupBegin }
function LoadGLTexture(const image: TEncodedImage;
  const Filter: TTextureFilter;
  const Wrap: TTextureWrap2D;
  const DDSForMipmaps: TDDSImage = nil;
  const GUITexture: boolean = false): TGLTextureId; overload;

function LoadGLTexture(const URL: string;
  const Filter: TTextureFilter;
  const Wrap: TTextureWrap2D;
  const DDSForMipmaps: TDDSImage = nil;
  const GUITexture: boolean = false): TGLTextureId; overload;
{ @groupEnd }

{ Load OpenGL texture into already reserved texture number.
  It uses existing OpenGL texture number (TextureId). Everything else
  works exactly the same as LoadGLTexture.

  You can also use this to set "default unnamed OpenGL texture" parameters
  by passing TextureId = 0.

  @raises(ETextureLoadError Raised in the same situations as LoadGLTexture.)

  @raises(EImageClassNotSupportedForOpenGL When Image class is not supported
    by OpenGL.)

  @groupBegin }
procedure LoadGLGeneratedTexture(const TextureId: TGLTextureId; const image: TEncodedImage;
  Filter: TTextureFilter;
  const Wrap: TTextureWrap2D;
  const DDSForMipmaps: TDDSImage = nil;
  const GUITexture: boolean = false); overload;
{ @groupEnd }

{ Decompress S3TC image by loading it to temporary OpenGL texture and
  reading back. So this internally uses current OpenGL context.

  @raises(ECannotLoadS3TCTexture If cannot decompress S3TC, for example
    because we cannot load to OpenGL this S3TC texture (because OpenGL S3TC
    extensions are not available, or such).) }
function GLDecompressS3TC(Image: TS3TCImage): TCastleImage;

{$endif read_interface}

{$ifdef read_implementation}

{ ----------------------------------------------------------------------
  Adjusting image size for 2D texture. }

function ResizeToTextureSize(const Size: Cardinal; const Sizing: TTextureSizing): Cardinal;
begin
  { make sure it's power of 2, if necessary }
  if ((Sizing = tsAny) and GLFeatures.TextureNonPowerOfTwo) or
    IsPowerOf2(Size) then
    Result := Size else
    Result := 1 shl (Biggest2Exponent(Size) + 1);

  { apply GLTextureScale }
  if (Sizing = tsScalablePowerOf2) and
     (GLTextureScale > 1) and
     (Result > GLTextureMinSize) then
    Result := Max(Result shr (GLTextureScale - 1), GLTextureMinSize);

  { honour GLFeatures.MaxTextureSize }
  if Result > GLFeatures.MaxTextureSize then
    Result := GLFeatures.MaxTextureSize;
end;

function ResizeToTextureSize(const r: TCastleImage; const Sizing: TTextureSizing): TCastleImage;
var
  NewWidth, NewHeight: Cardinal;
begin
  NewWidth  := ResizeToTextureSize(R.Width , Sizing);
  NewHeight := ResizeToTextureSize(R.Height, Sizing);

  if Log then
    WritelnLog('Textures', Format('Resizing 2D texture from %dx%d to %dx%d',
      [R.Width, R.Height, NewWidth, NewHeight]));

  result := r.MakeResized(NewWidth, NewHeight, riBilinear);
end;

function IsTextureSized(const Width, Height: Cardinal;
  const Sizing: TTextureSizing): boolean;
begin
  Result :=
    (Width  = ResizeToTextureSize(Width , Sizing)) and
    (Height = ResizeToTextureSize(Height, Sizing));
end;

function IsTextureSized(const r: TEncodedImage; const Sizing: TTextureSizing): boolean;
begin
  Result := IsTextureSized(r.Width, r.Height, Sizing);
end;

procedure ResizeForTextureSize(var r: TCastleImage; const Sizing: TTextureSizing);
var
  newR: TCastleImage;
begin
  if not IsTextureSized(r, Sizing) then
  begin
    newR := ResizeToTextureSize(r, Sizing);
    FreeAndNil(r);
    r := newR;
  end;
end;

{ 2D texture loading --------------------------------------------------------- }

function LoadGLTexture(const image: TEncodedImage;
  const Filter: TTextureFilter; const Wrap: TTextureWrap2D;
  const DDSForMipmaps: TDDSImage; const GUITexture: boolean): TGLTextureId;
begin
  glGenTextures(1, @result);
  LoadGLGeneratedTexture(result, image, Filter, Wrap, DDSForMipmaps, GUITexture);
end;

function LoadGLTexture(const URL: string;
  const Filter: TTextureFilter; const Wrap: TTextureWrap2D;
  const DDSForMipmaps: TDDSImage; const GUITexture: boolean): TGLTextureId;
var
  Image: TEncodedImage;
begin
  Image := LoadTextureImage(URL);
  try
    Result := LoadGLTexture(Image, Filter, Wrap, DDSForMipmaps, GUITexture);
  finally Image.Free end;
end;

{$ifndef OpenGLES}
{ Load Image through glCompressedTexImage2DARB.
  This checks existence of OpenGL extensions for S3TC,
  and checks Image sizes.
  It also takes care of pixel packing, although actually nothing needs
  be done about it when using compressed textures.

  Level = 0 for base (not a mipmap sublevel) image.

  @raises(ECannotLoadS3TCTexture If texture size is bad or OpenGL S3TC
    extensions are missing.) }
procedure glCompressedTextureImage2D(Image: TS3TCImage; Level: TGLint;
  var LoadedSize: Int64);
begin
  if not GLFeatures.TextureCompressionS3TC then
    raise ECannotLoadS3TCTexture.Create('Cannot load S3TC compressed textures: OpenGL doesn''t support one (or both) of ARB_texture_compression and EXT_texture_compression_s3tc extensions');

  if not IsTextureSized(Image, tsRequiredPowerOf2) then
    raise ECannotLoadS3TCTexture.CreateFmt('Cannot load S3TC compressed textures: texture size is %d x %d, it''s not correct for OpenGL, and we cannot resize on CPU compressed textures',
      [Image.Width, Image.Height]);

  { Pixel packing parameters (stuff changed by Before/AfterUnpackImage)
    doesn't affect loading compressed textures, as far as I understand.
    So no need to call it. }
  glCompressedTexImage2DARB(GL_TEXTURE_2D, Level, ImageGLInternalFormat(Image),
    Image.Width, Image.Height, 0, Image.Size,
    Image.RawPixels);
  LoadedSize += Image.Size;
end;
{$endif}

procedure CalculateImageRightSize(
  const Image: TCastleImage; const GUITexture: boolean;
  out ImageRightSize: TCastleImage; out ImageRightSizeFree: boolean);
var
  Sizing: TTextureSizing;
begin
  if GUITexture then
    Sizing := tsAny else
    Sizing := tsScalablePowerOf2;
  if IsTextureSized(Image, Sizing) then
  begin
    ImageRightSize := Image;
    ImageRightSizeFree := false;
  end else
  begin
    ImageRightSize := ResizeToTextureSize(Image, Sizing);
    ImageRightSizeFree := true;
  end;
end;

procedure LoadGLGeneratedTexture(const TextureId: TGLTextureId;
  const Image: TEncodedImage; Filter: TTextureFilter; const Wrap: TTextureWrap2D;
  const DDSForMipmaps: TDDSImage; const GUITexture: boolean);
var
  ImageInternalFormat: TGLuint;
  ImageFormat: TGLuint;
  LoadedSize: Int64;

  { Calls glTexImage2D for given image.
    Takes care of OpenGL unpacking (alignment etc.).
    Assumes that image already has the right size
    (power of 2, within OpenGL required sizes).
    Level = 0 for base (not a mipmap sublevel) image. }
  procedure glTexImage2DImage(Image: TCastleImage; Level: TGLint);
  var
    UnpackData: TUnpackNotAlignedData;
  begin
    { Nawet jesli ladujemy obrazek o ktorym wiemy ze ma wymiary dobre
      dla glTexImage2d, musimy zadbac o jego aligment : bo co by bylo
      jesli tekstura ma szerokosc 1 lub 2  ?
      Poza tym, planuje dodac tutaj robienie borderow dla tekstury, a wtedy
      wymiar dobry dla glTexImage2d to rownie dobrze 2^n+2 (a wiec prawie zawsze
      niepodzielne na 4). }
    BeforeUnpackImage(UnpackData, Image);
    try
      {$ifndef OpenGLES}
      { Workaround Mesa 7.9-devel bug (at least with Intel DRI,
        on Ubuntu 10.10, observed on domek): glTexImage2D accidentaly
        enables GL_TEXTURE_2D. }
      if GLVersion.Mesa then glPushAttrib(GL_ENABLE_BIT);
      {$endif}

      glTexImage2D(GL_TEXTURE_2D, Level, ImageInternalFormat,
        Image.Width, Image.Height, 0, ImageFormat, ImageGLType(Image),
        Image.RawPixels);
      LoadedSize += Image.Size;

      {$ifndef OpenGLES}
      if GLVersion.Mesa then glPopAttrib;
      {$endif}
    finally AfterUnpackImage(UnpackData, Image) end;
  end;

  procedure LoadNormal(const image: TCastleImage);
  begin
    glTexImage2DImage(Image, 0);
  end;

  {$ifndef OpenGLES}
  { Check should we load mipmaps from DDS. Load them, if yes.

    If LoadBase this also loads the base image (mipmap level 0).

    Note that I observed a bug on NVidia GeForce FX 5200, with various driver
    versions on both Linux 32 bit, 64 bit, and Windows 32 bit:
    you cannot load the base texture level (0) *after* loading the mipmaps.
    Doing so results in mipmaps being ignored, and seemingly GL_NEAREST
    minification filtering used (ignoring our set Filter.Minification).
    This could be easily observed with
    demo_models/x3d/tex_visualize_mipmaps.x3dv,
    switching to viewpoint like "Mipmaps from DDS" or "Colored mipmaps from DDS"
    --- you could clearly see that mipmaps are ignored and ugly nearest filtering
    gets used.
    Using LoadBase automatically workarounds this. }
  function LoadMipmapsFromDDS(DDS: TDDSImage; LoadBase: boolean): boolean;
  var
    I, FromLevel: Integer;
    ImageRightSize: TCastleImage;
    ImageRightSizeFree: boolean;
  begin
    Result := (DDS <> nil) and DDS.Mipmaps;
    if Result and (DDS.DDSType <> dtTexture) then
    begin
      OnWarning(wtMinor, 'Texture', 'DDS image contains mipmaps, but not for 2D texture');
      Result := false;
    end;

    if Result and (not GLFeatures.Version_1_2) then
    begin
      OnWarning(wtMinor, 'Texture', 'Cannot load DDS image containing mipmaps, because OpenGL 1.2 not available (GL_TEXTURE_MAX_LEVEL not available)');
      Result := false;
    end;

    if Result then
    begin
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, DDS.MipmapsCount - 1);
      if LoadBase then
        FromLevel := 0 else
        FromLevel := 1;
      for I := FromLevel to DDS.MipmapsCount - 1 do
        if DDS.Images[I] is TCastleImage then
        begin
          CalculateImageRightSize(TCastleImage(DDS.Images[I]), GUITexture,
            ImageRightSize, ImageRightSizeFree);
          try
            glTexImage2DImage(ImageRightSize, I);
          finally
            if ImageRightSizeFree then
              FreeAndNil(ImageRightSize) else
              ImageRightSize := nil;
          end;
        end else
        if DDS.Images[I] is TS3TCImage then
          glCompressedTextureImage2D(TS3TCImage(DDS.Images[I]), I, LoadedSize) else
          raise EInvalidImageForOpenGLTexture.CreateFmt('Cannot load to OpenGL texture image class %s', [Image.ClassName]);
    end;
  end;

  { Calls gluBuild2DMipmaps for given image.
    Takes care of OpenGL unpacking (alignment etc.).
    gluBuild2DMipmaps doesn't require size to be a power of 2, so no problems
    here. }
  procedure gluBuild2DMipmapsImage(Image: TCastleImage);
  var
    UnpackData: TUnpackNotAlignedData;
  begin
    BeforeUnpackImage(UnpackData, Image);
    try
      gluBuild2DMipmaps(GL_TEXTURE_2D, ImageInternalFormat,
        Image.Width, Image.Height, ImageFormat, ImageGLType(Image),
        Image.RawPixels);
      LoadedSize += TTextureMemoryProfiler.MipmapsSize(Image);
    finally AfterUnpackImage(UnpackData, Image) end;
  end;
  {$endif}

  procedure LoadMipmapped(const image: TCastleImage);
  begin
    {$ifndef OpenGLES}
    if not LoadMipmapsFromDDS(DDSForMipmaps, true) then
    if HasGenerateMipmap then
    begin
    {$endif}
      glTexImage2DImage(Image, 0);
      { hardware-accelerated mipmap generation }
      GenerateMipmap(GL_TEXTURE_2D);
      LoadedSize += TTextureMemoryProfiler.MipmapsSize(Image);
    {$ifndef OpenGLES}
    end else
      gluBuild2DMipmapsImage(Image);
    {$endif}
  end;

var
  ImageRightSize: TCastleImage;
  ImageRightSizeFree: boolean;
begin
  if GUITexture then
    Filter.DisableMipmaps;

  LoadedSize := 0;

  { bind the texture, set min, mag filters and wrap parameters }
  glBindTexture(GL_TEXTURE_2D, TextureId);
  SetTextureFilter(GL_TEXTURE_2D, Filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap[1]);

  { give the texture data }
  if Image is TCastleImage then
  begin
    ImageInternalFormat := ImageGLInternalFormat(Image);
    ImageFormat := ImageGLFormat(TCastleImage(Image));

    CalculateImageRightSize(TCastleImage(Image), GUITexture,
      ImageRightSize, ImageRightSizeFree);
    try
      { Load uncompressed }
      if Filter.NeedsMipmaps then
        LoadMipmapped(ImageRightSize) else
        LoadNormal(ImageRightSize);

      TextureMemoryProfiler.Allocate(TextureId, Image.URL, Image.ClassName, LoadedSize,
        Filter.NeedsMipmaps, ImageRightSize.Width, ImageRightSize.Height, 1);
    finally
      if ImageRightSizeFree then
        FreeAndNil(ImageRightSize) else
        ImageRightSize := nil;
    end;
  end else
  {$ifndef OpenGLES}
  if Image is TS3TCImage then
  begin
    { Load compressed }
    glCompressedTextureImage2D(TS3TCImage(Image), 0, LoadedSize);

    if Filter.NeedsMipmaps then
    begin
      if not LoadMipmapsFromDDS(DDSForMipmaps, false) then
      try
        GenerateMipmap(GL_TEXTURE_2D);
        LoadedSize += TTextureMemoryProfiler.MipmapsSize(Image);
      except
        on E: EGenerateMipmapNotAvailable do
        begin
          { Update GL_TEXTURE_MIN_FILTER, since we already initialized it earlier. }
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          OnWarning(wtMinor, 'Texture', 'Creating mipmaps for S3TC compressed textures requires GenerateMipmap functionality, will fallback to GL_LINEAR minification: ' + E.Message);
        end;
      end;
    end;

    TextureMemoryProfiler.Allocate(TextureId, Image.URL, Image.ClassName, LoadedSize,
      Filter.NeedsMipmaps, Image.Width, Image.Height, 1);
  end else
  {$endif}
    raise EInvalidImageForOpenGLTexture.CreateFmt('Cannot load to OpenGL texture image class %s', [Image.ClassName]);
end;

{ GLDecompressS3TC ------------------------------------------------------------- }

function GLDecompressS3TC(Image: TS3TCImage): TCastleImage;

{$ifndef OpenGLES}
var
  Tex: TGLTextureId;
  PackData: TPackNotAlignedData;
  LoadedSize: Int64;
begin
  glGenTextures(1, @Tex);
  glBindTexture(GL_TEXTURE_2D, Tex);

  { Testcase that fails on Radeon chantal (ATI Radeon X1600) Linux:
    demo_models/textures/marble_with_mipmaps_s3tc.dds

    No problem on NVidia (fpc 2.2.2 kocury/linux/32, fpc 2.2.4 kocury/linux/32),
    and no problem on Mac OS X with the same GPU (also chantal, 32bit, fpc 2.2.4).
    So I'm assuming it's fglrx-specific bug. }
  if GLVersion.Fglrx and ( (Image.Width < 4) or (Image.Height < 4) ) then
    raise ECannotDecompressS3TC.CreateFmt('Cannot decompress S3TC texture: fglrx (proprietary Radeon drivers on Linux) may awfully crash when one of texture sizes is smaller than 4, and your texture size is %d x %d',
      [Image.Width, Image.Height]);

  try
    LoadedSize := 0;
    glCompressedTextureImage2D(Image, 0, LoadedSize);
    TextureMemoryProfiler.Allocate(Tex, Image.URL, Image.ClassName,
      LoadedSize, false, Image.Width, Image.Height, 1);
  except
    { catch ECannotLoadS3TCTexture and change it to ECannotDecompressS3TC }
    on E: ECannotLoadS3TCTexture do
      raise ECannotDecompressS3TC.Create('Cannot decompress S3TC texture: ' + E.Message);
  end;

  case Image.Compression of
    s3tcDxt1_RGB: Result := TRGBImage.Create(Image.Width, Image.Height, Image.Depth);
    s3tcDxt1_RGBA,
    s3tcDxt3,
    s3tcDxt5: Result := TRGBAlphaImage.Create(Image.Width, Image.Height, Image.Depth);
    else raise EInternalError.Create('GLDecompressS3TC-Compression?');
  end;

  BeforePackImage(PackData, Result);
  try
    glGetTexImage(GL_TEXTURE_2D, 0,
      ImageGLFormat(Result), ImageGLType(Result), Result.RawPixels);
  finally AfterPackImage(PackData, Result) end;

  glFreeTexture(Tex);
end;

{$else}
begin
  raise ECannotDecompressS3TC.Create('Cannot decompress S3TC texture on OpenGL ES');
  Result := nil; // get rid of warning
end;
{$endif}

{$endif read_implementation}