{ -*- buffer-read-only: t -*-

  Copyright 2015-2017 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ Automatically generated node properties.

  Do not edit this file manually!
  To add new properties:
  - add them to the text files in nodes_specification/components/ ,
  - and regenerate include files by running
    nodes_specification/x3d-nodes-to-pascal/x3d-nodes-to-pascal.lpr .

  The documentation for properties should go to x3dnodes_documentation.txt . }

{$ifdef read_interface}

  strict private
    function GetAmbientIntensity: Single;
    procedure SetAmbientIntensity(const Value: Single);
    function GetBackAmbientIntensity: Single;
    procedure SetBackAmbientIntensity(const Value: Single);
    function GetBackDiffuseColor: TCastleColorRGB;
    procedure SetBackDiffuseColor(const Value: TCastleColorRGB);
    function GetBackEmissiveColor: TCastleColorRGB;
    procedure SetBackEmissiveColor(const Value: TCastleColorRGB);
    function GetBackShininess: Single;
    procedure SetBackShininess(const Value: Single);
    function GetBackSpecularColor: TCastleColorRGB;
    procedure SetBackSpecularColor(const Value: TCastleColorRGB);
    function GetBackTransparency: Single;
    procedure SetBackTransparency(const Value: Single);
    function GetDiffuseColor: TCastleColorRGB;
    procedure SetDiffuseColor(const Value: TCastleColorRGB);
    function GetEmissiveColor: TCastleColorRGB;
    procedure SetEmissiveColor(const Value: TCastleColorRGB);
    function GetShininess: Single;
    procedure SetShininess(const Value: Single);
    function GetSeparateBackColor: boolean;
    procedure SetSeparateBackColor(const Value: boolean);
    function GetSpecularColor: TCastleColorRGB;
    procedure SetSpecularColor(const Value: TCastleColorRGB);
    function GetTransparency: Single;
    procedure SetTransparency(const Value: Single);
  public
    property AmbientIntensity: Single read GetAmbientIntensity write SetAmbientIntensity;
    property BackAmbientIntensity: Single read GetBackAmbientIntensity write SetBackAmbientIntensity;
    property BackDiffuseColor: TCastleColorRGB read GetBackDiffuseColor write SetBackDiffuseColor;
    property BackEmissiveColor: TCastleColorRGB read GetBackEmissiveColor write SetBackEmissiveColor;
    property BackShininess: Single read GetBackShininess write SetBackShininess;
    property BackSpecularColor: TCastleColorRGB read GetBackSpecularColor write SetBackSpecularColor;
    property BackTransparency: Single read GetBackTransparency write SetBackTransparency;
    property DiffuseColor: TCastleColorRGB read GetDiffuseColor write SetDiffuseColor;
    property EmissiveColor: TCastleColorRGB read GetEmissiveColor write SetEmissiveColor;
    property Shininess: Single read GetShininess write SetShininess;
    property SeparateBackColor: boolean read GetSeparateBackColor write SetSeparateBackColor;
    property SpecularColor: TCastleColorRGB read GetSpecularColor write SetSpecularColor;
    property Transparency: Single read GetTransparency write SetTransparency;

{$endif read_interface}

{$ifdef read_implementation}

{ TTwoSidedMaterialNode ----------------------------------------------- }

function TTwoSidedMaterialNode.GetAmbientIntensity: Single;
begin
  Result := FdAmbientIntensity.Value;
end;

procedure TTwoSidedMaterialNode.SetAmbientIntensity(const Value: Single);
begin
  FdAmbientIntensity.Send(Value);
end;

function TTwoSidedMaterialNode.GetBackAmbientIntensity: Single;
begin
  Result := FdBackAmbientIntensity.Value;
end;

procedure TTwoSidedMaterialNode.SetBackAmbientIntensity(const Value: Single);
begin
  FdBackAmbientIntensity.Send(Value);
end;

function TTwoSidedMaterialNode.GetBackDiffuseColor: TCastleColorRGB;
begin
  Result := FdBackDiffuseColor.Value;
end;

procedure TTwoSidedMaterialNode.SetBackDiffuseColor(const Value: TCastleColorRGB);
begin
  FdBackDiffuseColor.Send(Value);
end;

function TTwoSidedMaterialNode.GetBackEmissiveColor: TCastleColorRGB;
begin
  Result := FdBackEmissiveColor.Value;
end;

procedure TTwoSidedMaterialNode.SetBackEmissiveColor(const Value: TCastleColorRGB);
begin
  FdBackEmissiveColor.Send(Value);
end;

function TTwoSidedMaterialNode.GetBackShininess: Single;
begin
  Result := FdBackShininess.Value;
end;

procedure TTwoSidedMaterialNode.SetBackShininess(const Value: Single);
begin
  FdBackShininess.Send(Value);
end;

function TTwoSidedMaterialNode.GetBackSpecularColor: TCastleColorRGB;
begin
  Result := FdBackSpecularColor.Value;
end;

procedure TTwoSidedMaterialNode.SetBackSpecularColor(const Value: TCastleColorRGB);
begin
  FdBackSpecularColor.Send(Value);
end;

function TTwoSidedMaterialNode.GetBackTransparency: Single;
begin
  Result := FdBackTransparency.Value;
end;

procedure TTwoSidedMaterialNode.SetBackTransparency(const Value: Single);
begin
  FdBackTransparency.Send(Value);
end;

function TTwoSidedMaterialNode.GetDiffuseColor: TCastleColorRGB;
begin
  Result := FdDiffuseColor.Value;
end;

procedure TTwoSidedMaterialNode.SetDiffuseColor(const Value: TCastleColorRGB);
begin
  FdDiffuseColor.Send(Value);
end;

function TTwoSidedMaterialNode.GetEmissiveColor: TCastleColorRGB;
begin
  Result := FdEmissiveColor.Value;
end;

procedure TTwoSidedMaterialNode.SetEmissiveColor(const Value: TCastleColorRGB);
begin
  FdEmissiveColor.Send(Value);
end;

function TTwoSidedMaterialNode.GetShininess: Single;
begin
  Result := FdShininess.Value;
end;

procedure TTwoSidedMaterialNode.SetShininess(const Value: Single);
begin
  FdShininess.Send(Value);
end;

function TTwoSidedMaterialNode.GetSeparateBackColor: boolean;
begin
  Result := FdSeparateBackColor.Value;
end;

procedure TTwoSidedMaterialNode.SetSeparateBackColor(const Value: boolean);
begin
  FdSeparateBackColor.Send(Value);
end;

function TTwoSidedMaterialNode.GetSpecularColor: TCastleColorRGB;
begin
  Result := FdSpecularColor.Value;
end;

procedure TTwoSidedMaterialNode.SetSpecularColor(const Value: TCastleColorRGB);
begin
  FdSpecularColor.Send(Value);
end;

function TTwoSidedMaterialNode.GetTransparency: Single;
begin
  Result := FdTransparency.Value;
end;

procedure TTwoSidedMaterialNode.SetTransparency(const Value: Single);
begin
  FdTransparency.Send(Value);
end;

{$endif read_implementation}
