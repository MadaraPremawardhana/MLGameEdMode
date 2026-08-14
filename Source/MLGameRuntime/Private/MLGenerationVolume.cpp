// Copyright. All rights reserved.
#include "MLGenerationVolume.h"
#include "MLModelAsset.h"
#include "MLGameBlueprintLibrary.h"
#include "MLGameRuntime.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

namespace
{
	const FName MLGeneratedTag(TEXT("MLGameEdMode_Generated"));
}

AMLGenerationVolume::AMLGenerationVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	Bounds->SetBoxExtent(FVector(500.f, 500.f, 200.f));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bounds->SetCollisionProfileName(TEXT("NoCollision"));
	Bounds->ShapeColor = FColor(80, 200, 255);
	RootComponent = Bounds;
}

void AMLGenerationVolume::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
	{
		Generate();
	}
}

void AMLGenerationVolume::Clear()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
	LastResults.Empty();
}

void AMLGenerationVolume::Generate()
{
	Clear();

	if (!ModelAsset)
	{
		UE_LOG(LogMLGame, Warning, TEXT("AMLGenerationVolume::Generate - '%s' has no ModelAsset assigned."), *GetName());
		return;
	}

	if (AssetPalette.Num() == 0)
	{
		UE_LOG(LogMLGame, Warning, TEXT("AMLGenerationVolume::Generate - '%s' has an empty AssetPalette."), *GetName());
		return;
	}

	if (!Runner.LoadModel(ModelAsset))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FRandomStream RandomStream(RandomSeed);

	const FVector BoxExtent = Bounds->GetScaledBoxExtent();
	const FVector BoxOrigin = Bounds->GetComponentLocation();
	const int32 GridX = FMath::Max(1, GridResolution.X);
	const int32 GridY = FMath::Max(1, GridResolution.Y);

	LastResults.Reserve(GridX * GridY);

	for (int32 Y = 0; Y < GridY; ++Y)
	{
		for (int32 X = 0; X < GridX; ++X)
		{
			const float NormX = GridX > 1 ? (float)X / (GridX - 1) : 0.5f;
			const float NormY = GridY > 1 ? (float)Y / (GridY - 1) : 0.5f;

			TMap<FName, float> Inputs = ExtraInputFeatures;
			Inputs.Add(TEXT("NormX"), NormX);
			Inputs.Add(TEXT("NormY"), NormY);

			TArray<float> FlatInputs;
			FlatInputs.Reserve(ModelAsset->InputFeatureNames.Num());
			for (const FName& FeatureName : ModelAsset->InputFeatureNames)
			{
				const float* Value = Inputs.Find(FeatureName);
				FlatInputs.Add(Value ? *Value : 0.f);
			}

			TArray<float> RawScores;
			if (!Runner.RunInference(FlatInputs, RawScores))
			{
				continue;
			}

			const TArray<float> Probabilities = UMLGameBlueprintLibrary::SoftmaxNormalize(RawScores);

			int32 SelectedIndex = INDEX_NONE;
			float SelectedConfidence = 0.f;

			if (bUseWeightedRandomSelection)
			{
				const float Roll = RandomStream.FRand();
				float Accum = 0.f;
				for (int32 Index = 0; Index < Probabilities.Num(); ++Index)
				{
					Accum += Probabilities[Index];
					if (Roll <= Accum)
					{
						SelectedIndex = Index;
						SelectedConfidence = Probabilities[Index];
						break;
					}
				}
				if (SelectedIndex == INDEX_NONE && Probabilities.Num() > 0)
				{
					SelectedIndex = Probabilities.Num() - 1;
					SelectedConfidence = Probabilities.Last();
				}
			}
			else
			{
				for (int32 Index = 0; Index < Probabilities.Num(); ++Index)
				{
					if (Probabilities[Index] > SelectedConfidence)
					{
						SelectedConfidence = Probabilities[Index];
						SelectedIndex = Index;
					}
				}
			}

			FMLGenerationCellResult CellResult;
			CellResult.GridCoord = FIntPoint(X, Y);
			CellResult.RawScores = RawScores;
			CellResult.SelectedPaletteIndex = SelectedIndex;
			CellResult.Confidence = SelectedConfidence;

			FVector CellLocation = BoxOrigin + FVector((NormX - 0.5f) * 2.f * BoxExtent.X, (NormY - 0.5f) * 2.f * BoxExtent.Y, BoxOrigin.Z);

			if (bTraceToGround)
			{
				const FVector TraceStart = FVector(CellLocation.X, CellLocation.Y, BoxOrigin.Z + BoxExtent.Z);
				const FVector TraceEnd = FVector(CellLocation.X, CellLocation.Y, BoxOrigin.Z - BoxExtent.Z);
				FHitResult Hit;
				if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
				{
					CellLocation = Hit.Location;
				}
			}

			CellResult.WorldLocation = CellLocation;
			LastResults.Add(CellResult);

			if (SelectedIndex == INDEX_NONE || !AssetPalette.IsValidIndex(SelectedIndex))
			{
				continue;
			}
			if (SelectedConfidence < MinConfidenceToSpawn)
			{
				continue;
			}

			const FMLPaletteEntry& Entry = AssetPalette[SelectedIndex];
			if (Entry.ActorClass.IsNull())
			{
				continue; // this palette entry deliberately means "spawn nothing"
			}

			UClass* ClassToSpawn = Entry.ActorClass.LoadSynchronous();
			if (!ClassToSpawn)
			{
				continue;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AActor* NewActor = World->SpawnActor<AActor>(ClassToSpawn, CellLocation, FRotator(0.f, RandomStream.FRandRange(0.f, 360.f), 0.f), SpawnParams);
			if (NewActor)
			{
				const float Scale = RandomStream.FRandRange(Entry.UniformScaleRange.X, Entry.UniformScaleRange.Y);
				NewActor->SetActorScale3D(FVector(Scale));
				NewActor->Tags.Add(MLGeneratedTag);
#if WITH_EDITOR
				NewActor->SetFolderPath(FName(*FString::Printf(TEXT("MLGenerated/%s"), *GetName())));
#endif
				NewActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
				SpawnedActors.Add(NewActor);
			}
		}
	}

	UE_LOG(LogMLGame, Log, TEXT("AMLGenerationVolume::Generate - '%s' spawned %d actors across a %dx%d grid."), *GetName(), SpawnedActors.Num(), GridX, GridY);
}
