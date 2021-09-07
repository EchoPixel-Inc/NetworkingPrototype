#include "clientApp/clientApp.h"
#include "clientApp/camera.h"
#include "clientApp/autostereoscopicOpenGLRenderWindow.h"
#include "config/config.h"
#include "networking/connection.h"
#include "appcore/messages.h"
#include "appcore/serializationHelper.h"
#include "appcore/serializationTypes.h"
#include "interaction/interactor.h"
#include "widgets/volumeWidget.h"
#include "widgets/splineWidget.h"
#include "widgets/laserWidget.h"
#include "widgets/planeWidget.h"
#include "vtkUtils/vtkGeneralizedCallbackCommand.h"
#include "vtkUtils/vtkErrorObserver.h"
#include "display/displayInterface.h"
#include "tracking/trackingUtils.h"

#include <cereal/types/string.hpp>
#include <cereal/types/array.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/optional.hpp>

#include <vtkRenderWindow.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkMatrix4x4.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkAxesActor.h>
#include <vtkPropAssembly.h>
#include <vtkNew.h>

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>

#ifdef WIN32
#	include "Windows.h"
#endif

#include <QHostAddress>
#include <QTcpSocket>
#include <QStandardItem>
#include <QEvent>

#include <iostream>
#include <sstream>
#include <vector>

//==============================================================================
ClientApp::ClientApp()
{
	m_Interactor = vtkSmartPointer<Interactor>::New();

	auto display = DisplayInterface::getDefaultImplementation();
	// TODO: Throw exception if we don't even have a default display?
	// Also may want to relocate the logic for determining the type of render
	// window we have to create
	vtkNew<Camera> camera;

	if (display->getInfo().stereoType != DisplayInfo::StereoType::NonStereo) {
		camera->SetEyeSeparation(60.0);
		camera->UseOffAxisProjectionOn();

		if (display->getInfo().stereoType ==
			DisplayInfo::StereoType::TimeSequential) {
			m_RenderWindow =
				vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
			m_RenderWindow->SetStereoCapableWindow(true);
			m_RenderWindow->SetStereoRender(true);
			m_RenderWindow->SetStereoTypeToCrystalEyes();
		}
		else if (display->getInfo().manufacturer == "Barco") {
			m_RenderWindow =
				vtkSmartPointer<AutostereoscopicOpenGLRenderWindow>::New();
			m_RenderWindow->SetStereoTypeToSplitViewportHorizontal();
			m_RenderWindow->SetStereoRender(true);
		}
	}
	else {
		m_RenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		camera->UseOffAxisProjectionOff();
		camera->SetPosition(0.0, 0.0, 400.0);
		vtkNew<vtkInteractorStyleTrackballCamera> style;
		style->AutoAdjustCameraClippingRangeOff();
		m_Interactor->SetInteractorStyle(style);
		m_Interactor->Initialize();
		m_RenderWindow->SetStereoRender(false);
	}

	vtkNew<vtkRenderer> renderer;
	renderer->SetActiveCamera(camera);
	// renderer->UseDepthPeelingOn();
	// renderer->UseDepthPeelingForVolumesOn();
	renderer->SetUseFXAA(true);

	m_RenderWindow->LineSmoothingOn();
	m_RenderWindow->PolygonSmoothingOn();
	m_RenderWindow->SetMultiSamples(8);
	m_RenderWindow->SetInteractor(m_Interactor);
	m_RenderWindow->AddRenderer(renderer);

	m_TrackingManager.setInteractor(m_Interactor);

	vtkNew<vtkAxesActor> axes;
	m_OrientationMarker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
	m_OrientationMarker->SetOutlineColor(0.9300, 0.5700, 0.1300);
	m_OrientationMarker->SetOrientationMarker(axes);
	m_OrientationMarker->SetInteractor(m_RenderWindow->GetInteractor());
	m_OrientationMarker->SetViewport(0.0, 0.0, 0.25, 0.25);
	m_OrientationMarker->SetEnabled(1);
	m_OrientationMarker->InteractiveOff();

	auto callbackCommand =
		vtkSmartPointer<vtkGeneralizedCallbackCommand>::New();
	callbackCommand->setCallback([this](vtkObject* caller, unsigned long eid,
									 void* callData) { updateScreenPose(); });
	m_RenderWindow->AddObserver(vtkCommand::WindowResizeEvent, callbackCommand);

	auto renderCallbackCommand =
		vtkSmartPointer<vtkGeneralizedCallbackCommand>::New();
	renderCallbackCommand->setCallback(
		[](vtkObject* caller, unsigned long eid, void* callData) {
			auto renderer = static_cast<vtkRenderer*>(caller);
			double propBounds[6];
			renderer->ComputeVisiblePropBounds(propBounds);

			if (vtkMath::AreBoundsInitialized(propBounds)) {
				static_cast<Camera*>(renderer->GetActiveCamera())
					->ResetClippingRange(propBounds);
			}
		});
	renderer->AddObserver(vtkCommand::StartEvent, renderCallbackCommand);

	m_RenderTimer.setInterval(std::chrono::milliseconds(30));
	QObject::connect(&m_RenderTimer, &QTimer::timeout, this, [this] {
		if (m_RenderWindow->GetStereoRender()) {
			auto currentHeadPose = m_TrackingManager.getCurrentHeadPose();
			m_RenderWindow->GetRenderers()
				->GetFirstRenderer()
				->GetActiveCamera()
				->SetEyeTransformMatrix(currentHeadPose.data());

			DisplayInterface::getDefaultImplementation()->sendEyePositions(
				tracking::estimateEyePositionsFromHeadPose(currentHeadPose));
		}
		m_RenderWindow->Render();
	});

	m_MessageEncoder.setOnLaserUpdatedCallback(
		[this](const LaserUpdate& laserUpdate, IdType) {
			onLaserUpdated(laserUpdate);
		});

	m_MessageEncoder.setOnVolumeUpdatedCallback(
		[this](const VolumeUpdate& volumeUpdate, IdType) {
			onVolumeUpdated(volumeUpdate);
		});

	m_MessageEncoder.setOnWidgetUpdatedCallback(
		[this](const WidgetUpdate& widgetUpdate, IdType) {
			onWidgetUpdated(widgetUpdate);
		});

	m_MessageEncoder.setOnPlaneUpdatedCallback(
		[this](const PlaneUpdate& planeUpdate, IdType) {
			onPlaneUpdated(planeUpdate);
		});

	m_MessageEncoder.setOnCredentialsRequestedCallback(
		[this] { onCredentialsRequested(); });

	m_MessageEncoder.setOnAuthorizationSuccessCallback(
		[this](const PeerInfo& info) { onAuthorizationSucceeded(info); });

	m_MessageEncoder.setOnAuthorizationFailedCallback(
		[this] { onAuthorizationFailed(); });

	m_MessageEncoder.setOnPeerAddedCallback(
		[this](const PeerInfo& peerInfo) { onPeerAdded(peerInfo); });

	m_MessageEncoder.setOnPeerRemovedCallback(
		[this](const PeerInfo& peerInfo) { onPeerRemoved(peerInfo); });

	m_MessageEncoder.setOnFullStateUpdatedCallback(
		[this](
			const std::vector<PeerInfo>& peers, ApplicationObjects&& dataObjects) {
			onFullStateUpdated(peers, std::move(dataObjects));
		});
}
//==============================================================================

//==============================================================================
ClientApp::~ClientApp()
{
	if (m_ServerProcess &&
		(m_ServerProcess->state() != QProcess::ProcessState::NotRunning)) {
		m_ServerProcess->close();
	}
}
//==============================================================================

//==============================================================================
ClientApp& ClientApp::instance()
{
	static ClientApp clientApp;
	return clientApp;
}
//==============================================================================

//==============================================================================
void ClientApp::initGraphics()
{
	if (m_RenderTimer.isActive()) {
		return;
	}

	m_RenderWindow->Render();
	updateScreenPose();

	if (!m_RenderTimer.isActive()) {
		std::cout << "starting render timer" << std::endl;
		m_RenderTimer.start();
	}

	QTimer::singleShot(0, [] {
		std::cout << "syncing stereo buffers" << std::endl;
		DisplayInterface::getDefaultImplementation()->syncStereoBuffers();
	});
}
//==============================================================================

//==============================================================================
void ClientApp::initTracking()
{
	const auto& config = Config::getDefaultConfig();

	if (config.interactionDevice != "null") {
		try {
			m_TrackingManager.initializeInteractionDevice(
				config.interactionDevice);
		}
		catch (std::exception& e) {
			std::cerr << "Interaction device initialization failed: "
					  << e.what() << std::endl;
		}
	}

	if (config.headTarget != "null") {
		try {
			m_TrackingManager.initializeHeadTracking(config.headTarget);
		}
		catch (std::exception& e) {
			std::cerr << "Head tracking initialization failed: " << e.what()
					  << std::endl;
		}
	}
}
//==============================================================================

//==============================================================================
void ClientApp::resetVolume()
{
	if (auto& volumeWidget = m_ApplicationObjects.volume) {
		volumeWidget->setTransform(VolumeWidget::TransformType::Identity());
		sendMessage(m_MessageEncoder.createVolumeUpdateMsg(
			VolumeUpdate(VolumeUpdate::MessageType::INTERACTION_ENDED)));
	}

	if (auto& planeWidget = m_ApplicationObjects.cutplane) {
		planeWidget->setTransform(PlaneWidget::TransformType::Identity());
		sendMessage(m_MessageEncoder.createPlaneUpdateMsg(
			PlaneUpdate(PlaneUpdate::MessageType::INTERACTION_ENDED)));
	}
}
//==============================================================================

//==============================================================================
void ClientApp::createWidget()
{
	sendMessage(m_MessageEncoder.createWidgetUpdateMsg(
		WidgetUpdate{WidgetUpdate::MessageType::CREATE}));
}
//==============================================================================

//==============================================================================
void ClientApp::updateScreenPose()
{
	const auto& displayInfo =
		DisplayInterface::getDefaultImplementation()->getInfo();

	// don't worry about updating the screen pose if we're in a non-stereo
	// setting or display size information is unavailable
	if ((displayInfo.stereoType == DisplayInfo::StereoType::NonStereo) ||
		(!displayInfo.pixelSizeInMM.has_value())) {
		return;
	}

	auto screenWidth = m_RenderWindow->GetScreenSize()[0];
	auto screenHeight = m_RenderWindow->GetScreenSize()[1];

	auto viewportWidth = m_RenderWindow->GetSize()[0];
	auto viewportHeight = m_RenderWindow->GetSize()[1];

	auto xPosition = m_RenderWindow->GetPosition()[0];
	auto yPosition = m_RenderWindow->GetPosition()[1];

	auto pixelSizeX = displayInfo.pixelSizeInMM.value()[0];
	auto pixelSizeY = displayInfo.pixelSizeInMM.value()[1];

	common::TransformType::VectorType screenBottomLeft = {
		pixelSizeX * (xPosition - 0.5 * screenWidth),
		pixelSizeY * (0.5 * screenHeight - yPosition - viewportHeight), 0.0};

	common::TransformType::VectorType screenBottomRight = {
		screenBottomLeft[0] + pixelSizeX * viewportWidth, screenBottomLeft[1],
		0.0};

	common::TransformType::VectorType screenTopRight = {screenBottomRight[0],
		screenBottomRight[1] + pixelSizeY * viewportHeight, 0.0};

	auto screenPose =
		DisplayInterface::getDefaultImplementation()->getPhysicalPose();

	screenBottomLeft = screenPose * screenBottomLeft;
	screenBottomRight = screenPose * screenBottomRight;
	screenTopRight = screenPose * screenTopRight;

	auto camera =
		m_RenderWindow->GetRenderers()->GetFirstRenderer()->GetActiveCamera();

	camera->SetScreenBottomLeft(
		screenBottomLeft[0], screenBottomLeft[1], screenBottomLeft[2]);

	camera->SetScreenBottomRight(
		screenBottomRight[0], screenBottomRight[1], screenBottomRight[2]);

	camera->SetScreenTopRight(
		screenTopRight[0], screenTopRight[1], screenTopRight[2]);
}
//==============================================================================

//==============================================================================
void ClientApp::connectToHost(
	const QHostAddress& hostAddress, quint16 portNumber)
{
	m_Connection = std::make_unique<Connection>();

	QObject::connect(
		m_Connection.get(), &Connection::messageReceived, m_Connection.get(),
		[this](const auto& msg) { m_MessageEncoder.processMessage(msg); },
		Qt::AutoConnection);

	QObject::connect(
		m_Connection.get(), &Connection::disconnected, m_Connection.get(),
		[this] {
			std::cout << "Connection has been closed..." << std::endl;
			onDisconnected();
		},
		Qt::QueuedConnection);

	QObject::connect(
		m_Connection.get(), &Connection::error, m_Connection.get(),
		[this](const QString& errorMsg) {
			std::cerr << "Received error message: " << errorMsg.toStdString()
					  << std::endl;
			onDisconnected();
		},
		Qt::AutoConnection);

	std::cout << "Attempting to connect to: "
			  << hostAddress.toString().toStdString() << " (port " << portNumber
			  << ")" << std::endl;

	m_Connection->connectToServer(hostAddress, portNumber);
}
//==============================================================================

//==============================================================================
void ClientApp::closeSession()
{
	if (m_Connection) {
		m_Connection->close();
	}
}
//==============================================================================

//==============================================================================
void ClientApp::launchServerApp(
	const QHostAddress& hostAddress, quint16 portNumber)
{
	if ((m_ServerProcess &&
			(m_ServerProcess->state() != QProcess::NotRunning)) ||
		m_ClientId.has_value()) {
		std::cout << "server process already running or already connected as a "
					 "client to a different session"
				  << std::endl;
	}

	m_ServerProcess = std::make_unique<QProcess>();
#ifdef WIN32
	m_ServerProcess->setCreateProcessArgumentsModifier(
		[](QProcess::CreateProcessArguments* args) {
			args->flags |= CREATE_NEW_CONSOLE;
			args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
		});
#endif
	QStringList arguments;
	arguments << "-i" << hostAddress.toString();
	arguments << "-p" << QString::number(portNumber);

	m_ServerProcess->setProgram(
		QString::fromStdString(Config::getDefaultConfig().serverPath));
	m_ServerProcess->setArguments(arguments);

	QObject::connect(
		m_ServerProcess.get(), &QProcess::started, m_ServerProcess.get(),
		[this, hostAddress, portNumber] {
			std::cout << "Server process has started..." << std::endl;

			emit serverStarted(QPrivateSignal{});
		},
		Qt::AutoConnection);

	QObject::connect(
		m_ServerProcess.get(), &QProcess::errorOccurred, m_ServerProcess.get(),
		[this](QProcess::ProcessError error) {
			std::cerr << "An error occurred in the server process" << std::endl;
			switch (error) {
				case QProcess::Crashed: {
					emit serverFinished(QPrivateSignal{});
					break;
				}
				default: {
					emit serverError("server error", QPrivateSignal{});
				}
			}
		},
		Qt::AutoConnection);

	QObject::connect(
		m_ServerProcess.get(), &QProcess::stateChanged, m_ServerProcess.get(),
		[this](QProcess::ProcessState state) {
			emit serverStatusChanged(state, QPrivateSignal{});
		},
		Qt::AutoConnection);

	QObject::connect(
		m_ServerProcess.get(),
		QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		m_ServerProcess.get(),
		[this](int exitCode, QProcess::ExitStatus exitStatus) {
			emit serverFinished(QPrivateSignal{});
		},
		Qt::AutoConnection);

	m_ServerProcess->start();
}
//==============================================================================

//==============================================================================
auto ClientApp::getRenderWindow() const -> vtkRenderWindow*
{
	return m_RenderWindow;
}
//==============================================================================

//==============================================================================
auto ClientApp::getPeerConnectionModel() -> QStandardItemModel&
{
	return m_ConnectedPeerModel;
}
//==============================================================================

//==============================================================================
void ClientApp::setImageData(vtkSmartPointer<vtkImageData> imageData)
{
	assert(imageData);
	auto imageVolume = vtkSmartPointer<vtkVolume>::New();

	// hard-coded transfer function parameters for now...
	auto colorTransferFunction =
		vtkSmartPointer<vtkColorTransferFunction>::New();

	colorTransferFunction->RemoveAllPoints();
	colorTransferFunction->AddHSVPoint(-3020, 0, 0, 0, 0.5, 0);
	colorTransferFunction->AddHSVPoint(-305, -1, 0, 0, 0.5, 0);
	colorTransferFunction->AddHSVPoint(129, -1, 0, 0, 0.5, 0);
	colorTransferFunction->AddHSVPoint(130, 0.99575, 1, 0.615686, 0.5, 0);
	colorTransferFunction->AddHSVPoint(
		179, 0.0532222, 0.951995, 0.956863, 0.5, 0);
	colorTransferFunction->AddHSVPoint(272, 0, 0.395834, 0.886275, 0.5, 0);
	colorTransferFunction->AddHSVPoint(585, -1, 0, 0.968627, 0.5, 0);
	colorTransferFunction->AddHSVPoint(681, -1, 0, 1, 0.5, 0);
	colorTransferFunction->AddHSVPoint(3070, -1, 0, 1, 0.5, 0);
	colorTransferFunction->AddHSVPoint(3071, -1, 0, 1, 0.5, 0);

	auto scalarOpacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	scalarOpacityFunction->AddPoint(-3020, 0);
	scalarOpacityFunction->AddPoint(-305, 0);
	scalarOpacityFunction->AddPoint(129, 0);
	scalarOpacityFunction->AddPoint(130, 0.0982);
	scalarOpacityFunction->AddPoint(179, 0.67);
	scalarOpacityFunction->AddPoint(272, 0.812);
	scalarOpacityFunction->AddPoint(585, 0.866);
	scalarOpacityFunction->AddPoint(681, 1);
	scalarOpacityFunction->AddPoint(3070, 1);
	scalarOpacityFunction->AddPoint(3071, 1);

	auto volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetInputData(imageData);
	volumeMapper->SetRequestedRenderModeToGPU();
	volumeMapper->AutoAdjustSampleDistancesOn();
	volumeMapper->SetMaxMemoryFraction(0.9f);
	volumeMapper->SetMaxMemoryInBytes(2000000000);
	volumeMapper->SetBlendModeToComposite();

	auto volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(colorTransferFunction);
	volumeProperty->SetScalarOpacity(scalarOpacityFunction);
	volumeProperty->SetIndependentComponents(true);
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->SetShade(1);
	volumeProperty->SetAmbient(0.15);
	volumeProperty->SetSpecular(0.32);
	volumeProperty->SetDiffuse(.78);

	imageVolume->SetMapper(volumeMapper);
	imageVolume->SetProperty(volumeProperty);

	auto imageOrigin = imageData->GetOrigin();
	auto imageDimensions = imageData->GetDimensions();
	auto imageSpacing = imageData->GetSpacing();

	double pos[3] = {
		imageOrigin[0] + 0.5 * imageSpacing[0] * imageDimensions[0],
		imageOrigin[1] + 0.5 * imageSpacing[1] * imageDimensions[1],
		imageOrigin[2] + 0.5 * imageSpacing[2] * imageDimensions[2] };

	imageVolume->SetPosition(-pos[0], -pos[1], -pos[2]);

	auto mtx = vtkSmartPointer<vtkMatrix4x4>::New();
	mtx->Identity();

	imageVolume->SetUserMatrix(mtx);

	auto& volumeWidget = m_ApplicationObjects.volume;
	volumeWidget.reset(new VolumeWidget{ imageVolume });
	volumeWidget->setInteractor(
		Interactor::SafeDownCast(m_RenderWindow->GetInteractor()));
}
//==============================================================================

//==============================================================================
void ClientApp::calibrateInteractionDevice()
{
	m_TrackingManager.calibrateInteractionDevice();
}
//==============================================================================

//==============================================================================
void ClientApp::onCredentialsRequested()
{
	emit credentialsRequested(QPrivateSignal{});
}
//==============================================================================

//==============================================================================
void ClientApp::sendCredentials(
	const std::string& sessionCode, const std::string& nickname)
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(PeerCredentials{sessionCode, nickname});
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = MessageType::PEER_CREDENTIALS;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	if (m_Connection) {
		m_Connection->sendMessage(msg);
	}
}
//==============================================================================

//==============================================================================
void ClientApp::onAuthorizationSucceeded(const PeerInfo& peerInfo)
{
	m_ClientId = peerInfo.id;

	std::cout << "Setting own client id to: " << peerInfo.id << "\n";
	std::cout << "Alias is: " << peerInfo.alias << "\n";
	std::cout << "Color is:  "
			  << "(" << peerInfo.color[0] << ", " << peerInfo.color[1] << ", "
			  << peerInfo.color[2] << ")" << std::endl;

	emit connectionStarted(QPrivateSignal{});
}
//==============================================================================

//==============================================================================
void ClientApp::onAuthorizationFailed()
{
	emit connectionError(
		"Authorization by the server failed", QPrivateSignal{});
}
//==============================================================================

//==============================================================================
void ClientApp::onDisconnected()
{
	m_Connection.reset();
	m_ClientId = std::nullopt;

	if (m_ServerProcess &&
		(m_ServerProcess->state() != QProcess::ProcessState::NotRunning)) {
		m_ServerProcess->close();
	}

	m_ApplicationObjects.lasers.clear();
	m_ApplicationObjects.widgets.clear();
	m_ApplicationObjects.cutplane->setProcessEvents(false);
	m_ApplicationObjects.volume->setProcessEvents(false);

	m_ConnectedPeerModel.clear();

	emit connectionEnded(QPrivateSignal{});
}
//==============================================================================

//==============================================================================
void ClientApp::onPeerAdded(const PeerInfo& peerInfo)
{
	if (peerInfo.id == m_ClientId.value()) {
		return;
	}

	// add to model
	auto newPeerItem = new QStandardItem;
	newPeerItem->setData(QVariant::fromValue(peerInfo.id), Qt::UserRole + 1);
	newPeerItem->setData(
		QString::fromStdString(peerInfo.alias), Qt::DisplayRole);
	newPeerItem->setData(QColor::fromRgbF(peerInfo.color[0], peerInfo.color[1],
							 peerInfo.color[2]),
		Qt::DecorationRole);
	newPeerItem->setData(false, Qt::UserRole + 2);	// whether this user "owns"
	// the volume object

	m_ConnectedPeerModel.appendRow(newPeerItem);

	// add laser
	auto newLaser = std::make_unique<LaserWidget>();
	newLaser->setColor(peerInfo.color[0], peerInfo.color[1], peerInfo.color[2]);
	newLaser->setInteractor(m_Interactor);
	newLaser->setVisible(true);
	newLaser->setProcessEvents(false);

	QObject::connect(newLaser.get(), &LaserWidget::propertyUpdated,
		[this](const auto& propList) {
			// TODO: send the prop update
		});

	m_ApplicationObjects.lasers.insert({peerInfo.id, std::move(newLaser)});
}
//==============================================================================

//==============================================================================
void ClientApp::onPeerRemoved(const PeerInfo& peerInfo)
{
	// remove from model
	for (auto& index : m_ConnectedPeerModel.match(
			 m_ConnectedPeerModel.index(0, 0), Qt::UserRole + 1,
			 QVariant::fromValue(peerInfo.id), -1, Qt::MatchExactly)) {
		m_ConnectedPeerModel.removeRow(index.row());
	}

	// remove any associated laser
	m_ApplicationObjects.lasers.erase(peerInfo.id);
}
//==============================================================================

//==============================================================================
void ClientApp::onLaserUpdated(const LaserUpdate& laserUpdate)
{
	if (auto it = m_ApplicationObjects.lasers.find(laserUpdate.id);
		it != m_ApplicationObjects.lasers.end()) {
		it->second->updateProperties(laserUpdate.propList);
	}
}
//==============================================================================

//==============================================================================
void ClientApp::onVolumeUpdated(const VolumeUpdate& volumeUpdate)
{
	switch (volumeUpdate.msgType) {
		case VolumeUpdate::MessageType::PROPERTY_UPDATE: {
			m_ApplicationObjects.volume->updateProperties(
				volumeUpdate.propList);
			break;
		}
		case VolumeUpdate::MessageType::INTERACTION_STARTED: {
			for (auto& index :
				m_ConnectedPeerModel.match(m_ConnectedPeerModel.index(0, 0),
					Qt::UserRole + 1, QVariant::fromValue(volumeUpdate.id), -1,
					Qt::MatchExactly)) {
				m_ConnectedPeerModel.setData(index, true, Qt::UserRole + 2);
			}
			break;
		}
		case VolumeUpdate::MessageType::INTERACTION_ENDED: {
			for (auto& index :
				m_ConnectedPeerModel.match(m_ConnectedPeerModel.index(0, 0),
					Qt::UserRole + 1, QVariant::fromValue(volumeUpdate.id), -1,
					Qt::MatchExactly)) {
				m_ConnectedPeerModel.setData(index, false, Qt::UserRole + 2);
			}
			break;
		}
	}  // end switch
}
//==============================================================================

//==============================================================================
void ClientApp::onWidgetUpdated(const WidgetUpdate& widgetUpdate)
{
	switch (widgetUpdate.msgType) {
		case WidgetUpdate::MessageType::CREATE: {
			auto widget = std::make_unique<SplineWidget>();
			widget->setInteractor(m_Interactor);
			widget->setProcessEvents(true);

			QObject::connect(
				widget.get(), &SplineWidget::requestPropertyUpdate,
				widget.get(),
				[this, id = widgetUpdate.widgetId](
					const SplineWidget::PropertyListType& propList) {
					sendMessage(m_MessageEncoder.createWidgetUpdateMsg(
						WidgetUpdate{WidgetUpdate::MessageType::PROPERTY_UPDATE,
							id, propList}));
				},
				Qt::AutoConnection);

			QObject::connect(
				widget.get(), &SplineWidget::interactionStateChanged,
				widget.get(),
				[this, id = widgetUpdate.widgetId](
					SplineWidget::InteractionState newState,
					SplineWidget::InteractionState oldState) {
					if (newState != SplineWidget::InteractionState::ACTIVE) {
						sendMessage(
							m_MessageEncoder.createWidgetUpdateMsg(WidgetUpdate{
								WidgetUpdate::MessageType::INTERACTION_ENDED,
								id}));
					}
				},
				Qt::AutoConnection);

			QObject::connect(
				widget.get(), &SplineWidget::interactionStateChanged,
				widget.get(),
				[this](SplineWidget::InteractionState newState,
					SplineWidget::InteractionState oldState) {
					if ((newState == SplineWidget::InteractionState::ACTIVE) ||
						(newState ==
							SplineWidget::InteractionState::INTERSECTING)) {
						auto& laserMap = m_ApplicationObjects.lasers;
						if (auto it = laserMap.find(m_ClientId.value());
							it != laserMap.end()) {
							it->second->setHighlight(true);
						}
					}
					else if (newState ==
						SplineWidget::InteractionState::INACTIVE) {
						auto& laserMap = m_ApplicationObjects.lasers;
						if (auto it = laserMap.find(m_ClientId.value());
							it != laserMap.end()) {
							it->second->setHighlight(false);
						}
					}

					if (oldState == SplineWidget::InteractionState::DEFINING) {
						emit widgetPlacementEnded(QPrivateSignal{});
					}
				},
				Qt::AutoConnection);

			// our requested new widget has been accepted, so now we
			// can proceed with node placement, etc.
			if (widgetUpdate.ownerId == m_ClientId.value()) {
				widget->startInteractivePlacement();
			}

			m_ApplicationObjects.widgets.insert(
				{widgetUpdate.widgetId, std::move(widget)});

			break;
		}
		case WidgetUpdate::MessageType::DESTROY: {
			m_ApplicationObjects.widgets.erase(widgetUpdate.widgetId);

			break;
		}
		case WidgetUpdate::MessageType::PROPERTY_UPDATE: {
			auto& widgets = m_ApplicationObjects.widgets;
			if (auto it = widgets.find(widgetUpdate.widgetId);
				it != widgets.end()) {
				it->second->updateProperties(widgetUpdate.propList);
			}

			break;
		}
	}  // end switch
}
//==============================================================================

//==============================================================================
void ClientApp::onPlaneUpdated(const PlaneUpdate& planeUpdate)
{
	switch (planeUpdate.msgType) {
		case PlaneUpdate::MessageType::PROPERTY_UPDATE: {
			m_ApplicationObjects.cutplane->updateProperties(
				planeUpdate.propList);

			break;
		}
	}  // end switch
}
//==============================================================================

//==============================================================================
void ClientApp::onFullStateUpdated(
	const std::vector<PeerInfo>& peers, ApplicationObjects&& dataObjects)
{
	// cache the current volume
	vtkSmartPointer<vtkVolume> cachedVolume =
		m_ApplicationObjects.volume->getVolume();

	m_ApplicationObjects = std::move(dataObjects);

	// peer information
	m_ConnectedPeerModel.clear();

	for (const auto& peer : peers) {
		auto newPeerItem = new QStandardItem;
		newPeerItem->setData(QVariant::fromValue(peer.id), Qt::UserRole + 1);
		newPeerItem->setData(
			QColor::fromRgbF(peer.color[0], peer.color[1], peer.color[2]),
			Qt::DecorationRole);

		if (peer.id == m_ClientId.value()) {
			newPeerItem->setData(
				QString::fromStdString(peer.alias + " (Me)"), Qt::DisplayRole);
		}
		else {
			newPeerItem->setData(
				QString::fromStdString(peer.alias), Qt::DisplayRole);
		}

		m_ConnectedPeerModel.appendRow(newPeerItem);
	}

	// setup lasers -----------------------------------------------------------
	for (auto& [id, laser] : m_ApplicationObjects.lasers) {
		QObject::connect(
			laser.get(), &LaserWidget::requestPropertyUpdate, laser.get(),
			[this, id = id](const auto& propList) {
				sendMessage(
					m_MessageEncoder.createLaserUpdateMsg(LaserUpdate(propList)));
			},
			Qt::AutoConnection);

		laser->setVisible(true);
		laser->setInteractor(m_Interactor);
		if (id == m_ClientId.value()) {
			laser->setProcessEvents(true);
		}
		else {
			laser->setProcessEvents(false);
		}
	}

	// setup volume -----------------------------------------------------------
	m_ApplicationObjects.volume->setVolume(cachedVolume);
	m_ApplicationObjects.volume->setInteractor(m_Interactor);
	m_ApplicationObjects.volume->setProcessEvents(true);

	auto& volume = m_ApplicationObjects.volume;
	QObject::connect(
		volume.get(), &VolumeWidget::requestPropertyUpdate, volume.get(),
		[this](const VolumeWidget::PropertyListType& propList) {
			sendMessage(m_MessageEncoder.createVolumeUpdateMsg(VolumeUpdate(
				VolumeUpdate::MessageType::PROPERTY_UPDATE, propList)));
		},
		Qt::AutoConnection);

	QObject::connect(
		volume.get(), &VolumeWidget::interactionStateChanged, volume.get(),
		[this](VolumeWidget::InteractionState state) {
			if (state != VolumeWidget::InteractionState::ACTIVE) {
				sendMessage(m_MessageEncoder.createVolumeUpdateMsg(VolumeUpdate(
					VolumeUpdate::MessageType::INTERACTION_ENDED)));
			}
		},
		Qt::AutoConnection);

	QObject::connect(
		volume.get(), &VolumeWidget::interactionStateChanged, volume.get(),
		[this](VolumeWidget::InteractionState state) {
			if ((state == VolumeWidget::InteractionState::ACTIVE) ||
				(state == VolumeWidget::InteractionState::INTERSECTING)) {
				auto& laserMap = m_ApplicationObjects.lasers;
				if (auto it = laserMap.find(m_ClientId.value());
					it != laserMap.end()) {
					it->second->setHighlight(true);
				}
			}
			else if (state == VolumeWidget::InteractionState::INACTIVE) {
				auto& laserMap = m_ApplicationObjects.lasers;
				if (auto it = laserMap.find(m_ClientId.value());
					it != laserMap.end()) {
					it->second->setHighlight(false);
				}
			}
		},
		Qt::AutoConnection);

	// setup plane widget -----------------------------------------------------
	m_ApplicationObjects.cutplane->setInteractor(m_Interactor);
	m_ApplicationObjects.cutplane->setProcessEvents(true);

    m_ApplicationObjects.volume->removeAllClippingPlanes();
	m_ApplicationObjects.volume->addClippingPlane(
		m_ApplicationObjects.cutplane->getPlane());

	auto& planeWidget = m_ApplicationObjects.cutplane;
	QObject::connect(
		planeWidget.get(), &PlaneWidget::requestPropertyUpdate,
		planeWidget.get(),
		[this](const PlaneWidget::PropertyListType& propList) {
			sendMessage(m_MessageEncoder.createPlaneUpdateMsg(PlaneUpdate(
				PlaneUpdate::MessageType::PROPERTY_UPDATE, propList)));
		},
		Qt::AutoConnection);

	QObject::connect(
		planeWidget.get(), &PlaneWidget::interactionStateChanged,
		planeWidget.get(),
		[this](PlaneWidget::InteractionState state) {
			if (state != PlaneWidget::InteractionState::ACTIVE) {
				sendMessage(m_MessageEncoder.createPlaneUpdateMsg(
					PlaneUpdate(PlaneUpdate::MessageType::INTERACTION_ENDED)));
			}
		},
		Qt::AutoConnection);

	QObject::connect(
		planeWidget.get(), &PlaneWidget::interactionStateChanged,
		planeWidget.get(),
		[this](PlaneWidget::InteractionState state) {
			if ((state == PlaneWidget::InteractionState::ACTIVE) ||
				(state == PlaneWidget::InteractionState::INTERSECTING)) {
				auto& laserMap = m_ApplicationObjects.lasers;
				if (auto it = laserMap.find(m_ClientId.value());
					it != laserMap.end()) {
					it->second->setHighlight(true);
				}
			}
			else if (state == PlaneWidget::InteractionState::INACTIVE) {
				auto& laserMap = m_ApplicationObjects.lasers;
				if (auto it = laserMap.find(m_ClientId.value());
					it != laserMap.end()) {
					it->second->setHighlight(false);
				}
			}
		},
		Qt::AutoConnection);

	// setup widgets ---------------------------------------------------------
	for (auto& [widgetId, widget] : m_ApplicationObjects.widgets) {
		widget->setInteractor(m_Interactor);
		widget->setProcessEvents(true);

		QObject::connect(
			widget.get(), &SplineWidget::requestPropertyUpdate, widget.get(),
			[this, id = widgetId](
				const SplineWidget::PropertyListType& propList) {
				sendMessage(m_MessageEncoder.createWidgetUpdateMsg(WidgetUpdate{
					WidgetUpdate::MessageType::PROPERTY_UPDATE, id, propList}));
			},
			Qt::AutoConnection);

		QObject::connect(
			widget.get(), &SplineWidget::interactionStateChanged, widget.get(),
			[this, id = widgetId](SplineWidget::InteractionState newState,
				SplineWidget::InteractionState oldState) {
				if (newState != SplineWidget::InteractionState::ACTIVE) {
					sendMessage(m_MessageEncoder.createWidgetUpdateMsg(WidgetUpdate{
						WidgetUpdate::MessageType::INTERACTION_ENDED, id}));
				}
			},
			Qt::AutoConnection);

		QObject::connect(
			widget.get(), &SplineWidget::interactionStateChanged, widget.get(),
			[this](SplineWidget::InteractionState newState,
				SplineWidget::InteractionState oldState) {
				if ((newState == SplineWidget::InteractionState::ACTIVE) ||
					(newState ==
						SplineWidget::InteractionState::INTERSECTING)) {
					auto& laserMap = m_ApplicationObjects.lasers;
					if (auto it = laserMap.find(m_ClientId.value());
						it != laserMap.end()) {
						it->second->setHighlight(true);
					}
				}
				else if (newState == SplineWidget::InteractionState::INACTIVE) {
					auto& laserMap = m_ApplicationObjects.lasers;
					if (auto it = laserMap.find(m_ClientId.value());
						it != laserMap.end()) {
						it->second->setHighlight(false);
					}
				}

				if (oldState == SplineWidget::InteractionState::DEFINING) {
					emit widgetPlacementEnded(QPrivateSignal{});
				}
			},
			Qt::AutoConnection);
	}
}
//==============================================================================

//==============================================================================
void ClientApp::sendMessage(const NetworkMessage& msg)
{
	if (m_Connection) {
		m_Connection->sendMessage(msg);
	}
}
//==============================================================================