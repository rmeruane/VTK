/*
 * Copyright 2004 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */


#include "vtkIconGlyphFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkCxxRevisionMacro(vtkIconGlyphFilter, "1.2");
vtkStandardNewMacro(vtkIconGlyphFilter);

//-----------------------------------------------------------------------------
vtkIconGlyphFilter::vtkIconGlyphFilter()
{
  this->IconSize[0] = 1;
  this->IconSize[1] = 1;
  this->IconSheetSize[0] = 1;
  this->IconSheetSize[1] = 1;

  this->SetInputArrayToProcess(0, 0, 0, 
    vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
}

//-----------------------------------------------------------------------------
vtkIconGlyphFilter::~vtkIconGlyphFilter()
{
}

//-----------------------------------------------------------------------------
void vtkIconGlyphFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "IconSize: " << IconSize[0] << " " << IconSize[1] << endl;
  os << indent << "IconSheetSize: " << IconSheetSize[0] << " " << IconSheetSize[1] << endl;
}

//-----------------------------------------------------------------------------
int vtkIconGlyphFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                  vtkInformationVector **inputVector,
                                  vtkInformationVector *outputVector)
{
  // Get the information object.
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // Get the data objects.
  vtkPointSet *input = vtkPointSet::SafeDownCast(
                                     inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
                                    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numPoints = input->GetNumberOfPoints();

  if (numPoints <= 0)
    {
    // nothing to do...
    return 1;
    }

  vtkIntArray* scalars = vtkIntArray::SafeDownCast(
    this->GetInputArrayToProcess(0, inputVector));
  if (!scalars)
    {
    vtkErrorMacro("Input Scalars must be specified to index into the icon sheet.");
    return 0;
    }

  double point[3], textureCoord[2];
  double sheetXDim = this->IconSheetSize[0]/this->IconSize[0];
  double sheetYDim = this->IconSheetSize[1]/this->IconSize[1];
  int iconIndex = 0;
  int j, k;

  vtkPoints * outPoints = vtkPoints::New();
  outPoints->Allocate(4 * numPoints);

  vtkCellArray * outCells = vtkCellArray::New();
  outCells->Allocate(outCells->EstimateSize(numPoints, 4));

  vtkFloatArray *outTCoords = vtkFloatArray::New();
  outTCoords->SetNumberOfComponents(2);
  outTCoords->Allocate(8*numPoints);

  for(int i = 0; i < numPoints; i++)
    {
    iconIndex = scalars->GetValue(i);
    input->GetPoint(i, point);
    outPoints->InsertNextPoint(point[0] - 0.5, point[1] - 0.5, point[2]);

    this->IconConvertIndex(iconIndex, j, k);
    textureCoord[0] = j/sheetXDim;
    textureCoord[1] = k/sheetYDim;
    outTCoords->InsertTuple(i * 4, textureCoord);

    outPoints->InsertNextPoint(point[0] + 0.5, point[1] - 0.5, point[2]);

    this->IconConvertIndex(iconIndex, j, k);
    textureCoord[0] = (j + 1.0)/sheetXDim;
    textureCoord[1] = k/sheetYDim;
    outTCoords->InsertTuple(i * 4 + 1, textureCoord);

    outPoints->InsertNextPoint(point[0] + 0.5, point[1] + 0.5, point[2]);

    this->IconConvertIndex(iconIndex, j, k);
    textureCoord[0] = (j + 1.0)/sheetXDim;
    textureCoord[1] = (k + 1.0)/sheetYDim;
    outTCoords->InsertTuple(i * 4 + 2, textureCoord);

    outPoints->InsertNextPoint(point[0] - 0.5, point[1] + 0.5, point[2]);

    this->IconConvertIndex(iconIndex, j, k);
    textureCoord[0] = j/sheetXDim;
    textureCoord[1] = (k + 1.0)/sheetYDim;
    outTCoords->InsertTuple(i * 4 + 3, textureCoord);

    outCells->InsertNextCell(4);
    outCells->InsertCellPoint(i * 4);
    outCells->InsertCellPoint(i * 4 + 1);
    outCells->InsertCellPoint(i * 4 + 2);
    outCells->InsertCellPoint(i * 4 + 3);
    }

  output->SetPoints(outPoints);
  outPoints->Delete();

  outTCoords->SetName("TextureCoordinates");
  output->GetPointData()->SetTCoords(outTCoords);
  outTCoords->Delete();

  output->SetPolys(outCells);
  outCells->Delete();

  return 1;
}

