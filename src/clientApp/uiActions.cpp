#include "clientApp/uiActions.h"
#include "clientApp/clientApp.h"
#include "clientApp/peerConnectionWindow.h"
#include "clientApp/credentialsDialog.h"
#include "clientApp/networkSessionSelectionDialog.h"
#include "clientApp/networkSessionConnectionDialog.h"
#include "config/config.h"
#include "vtkUtils/vtkErrorObserver.h"

#include <QFileDialog>
#include <QMessageBox>

#include <vtkNew.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>

//==============================================================================
UIActions::UIActions(QWidget* parent) :
	parent{parent},
	peerConnectionsWindow{nullptr}
{
	openDICOMAction = new QAction("Open DICOM", parent);
	QObject::connect(openDICOMAction, &QAction::triggered, this,
		&UIActions::onLoadDICOM);

	resetVolumeAction = new QAction("Reset Volume", parent);
	resetVolumeAction->setShortcut(QKeySequence(Qt::Key_R));
	QObject::connect(resetVolumeAction, &QAction::triggered,
		parent, [] {
			ClientApp::instance().resetVolume();
		});

	startNetworkSessionAction = new QAction("Start Network Session", parent);
	QObject::connect(startNetworkSessionAction, &QAction::triggered,
		this, &UIActions::onNetworkSessionRequested);

	showPeerConnectionsAction = new QAction("Show Peer Connections", parent);
	showPeerConnectionsAction->setVisible(false);
	QObject::connect(showPeerConnectionsAction, &QAction::triggered,
		parent, [this] {
			peerConnectionsWindow->show();
		});

	createWidgetAction = new QAction("Create Widget", parent);
	createWidgetAction->setEnabled(false);
	QObject::connect(createWidgetAction, &QAction::triggered,
		parent, [] {
			ClientApp::instance().createWidget();
		});

	calibrateInteractionDeviceAction = new QAction("Calibrate Interaction\n Device");
	calibrateInteractionDeviceAction->setEnabled(true);
	QObject::connect(calibrateInteractionDeviceAction, &QAction::triggered,
		parent, [] {
			ClientApp::instance().calibrateInteractionDevice();
		});

	QObject::connect(
		&ClientApp::instance(), &ClientApp::credentialsRequested, parent, [this] {
			CredentialsDialog credentialsDialog(this->parent);
			credentialsDialog.setName(QString::fromStdString(
				Config::getDefaultConfig().defaultAlias));
			auto returnState = credentialsDialog.exec();

			if (returnState == QDialog::Accepted) {
				ClientApp::instance().sendCredentials(
					credentialsDialog.getSessionKey().toStdString(),
					credentialsDialog.getName().toStdString());
			}
			else {
				// Just close the connection if we don't want to provide
				// any credentials
				ClientApp::instance().closeSession();
			}
		});

	QObject::connect(&ClientApp::instance(), &ClientApp::connectionError, parent,
		[](const QString& errorMsg) {
			// TODO: show an error dialog
			std::cerr << "Got a 'connectionError' message: "
				<< errorMsg.toStdString() << std::endl;
		});

	QObject::connect(
		&ClientApp::instance(), &ClientApp::connectionStarted, parent, [this] {
			// TODO: change menu item visibility?
			std::cout << "MainWindow:: got a 'connectionStarted' message"
				<< std::endl;
			peerConnectionsWindow->show();
			showPeerConnectionsAction->setVisible(true);
			createWidgetAction->setEnabled(true);
			sessionSelectionDialog->setEnableJoinSession(false);
		});

	QObject::connect(
		&ClientApp::instance(), &ClientApp::connectionEnded, parent, [this] {
			std::cout << "MainWindow:: got a 'connectionEnded' message"
				<< std::endl;
			peerConnectionsWindow->hide();
			showPeerConnectionsAction->setVisible(false);
			sessionSelectionDialog->setEnableJoinSession(true);
		});

	QObject::connect(&ClientApp::instance(), &ClientApp::serverError, parent,
		[](const QString& errorMsg) {
			std::cerr << "MainWindow:: got a 'serverError' message: "
				<< errorMsg.toStdString() << std::endl;
			// TODO: show error message
		});

	QObject::connect(&ClientApp::instance(), &ClientApp::serverFinished,
		parent, [this] {
			sessionSelectionDialog->setEnableStartSession(true);
		});

	QObject::connect(&ClientApp::instance(), &ClientApp::widgetPlacementEnded,
		parent, [this] {
			createWidgetAction->setEnabled(true);
		}, Qt::AutoConnection);
}
//==============================================================================

//==============================================================================
UIActions::~UIActions() = default;
//==============================================================================

//==============================================================================
void UIActions::onNetworkSessionRequested()
{
	if (!peerConnectionsWindow) {
		peerConnectionsWindow = new PeerConnectionWindow(parent);
		peerConnectionsWindow->setWindowFlags(
			peerConnectionsWindow->windowFlags() | Qt::Tool);

		peerConnectionsWindow->setPeerModel(
			&ClientApp::instance().getPeerConnectionModel());
	}

	if (!sessionSelectionDialog) {
		sessionSelectionDialog = std::make_unique<NetworkSessionSelectionDialog>();
		sessionSelectionDialog->setEnableStartSession(true);
		sessionSelectionDialog->setEnableJoinSession(true);
	}

	if (sessionSelectionDialog->exec() == QDialog::Rejected) {
		return;
	}

	NetworkSessionConnectionDialog connectionDialog;
	if (connectionDialog.exec() == QDialog::Rejected) {
		return;
	}

	auto hostAddress = connectionDialog.getHostAddress();
	auto portNumber = connectionDialog.getPortNumber();

	using SessionType = NetworkSessionSelectionDialog::SessionType;
	auto sessionType = sessionSelectionDialog->getSessionSelection();

	if (sessionType == SessionType::Start) {
		QObject::disconnect(
			onServerStartedConnection);  // since we're dealing with a lambda,
										   // we need to invalidate any previous
										   // connections
		QObject::connect(&ClientApp::instance(), &ClientApp::serverStarted,
			this, [hostAddress, portNumber] {
				ClientApp::instance().connectToHost(hostAddress, portNumber);
			});

		sessionSelectionDialog->setEnableStartSession(false);
		ClientApp::instance().launchServerApp(hostAddress, portNumber);
	}
	else {
		ClientApp::instance().connectToHost(hostAddress, portNumber);
	}
}
//==============================================================================

//==============================================================================
void UIActions::onLoadDICOM()
{
	auto dicomDir = QFileDialog::getExistingDirectory(nullptr,
		QObject::tr("Select DICOM directory"), "C:/",
			QFileDialog::DontUseNativeDialog |
			QFileDialog::ShowDirsOnly |
			QFileDialog::DontResolveSymlinks);

	if (dicomDir.isEmpty()) {
		return;
	}

	auto imageReader = vtkSmartPointer<vtkDICOMImageReader>::New();
	imageReader->SetDirectoryName(dicomDir.toStdString().c_str());

	vtkNew<vtkErrorObserver> errorObserver;
	imageReader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
	imageReader->AddObserver(vtkCommand::WarningEvent, errorObserver);

	imageReader->Update();

	if (errorObserver->lastErrorMessage.has_value()) {
		QMessageBox msgBox;
		msgBox.setWindowTitle("Read DICOM Error");
		msgBox.setInformativeText(
			"Could not read DICOM files in the directory specified");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();

		std::cerr << errorObserver->lastErrorMessage.value() << std::endl;

		return;
	}
	else if (errorObserver->lastWarningMessage.has_value()) {
		std::cerr << errorObserver->lastWarningMessage.value() << std::endl;
	}

	auto imageData = vtkSmartPointer<vtkImageData>::New();
	imageData = imageReader->GetOutput();

	if (imageData) {
		ClientApp::instance().setImageData(imageData);
	}
	else {
		std::cerr << "No image data to set" << std::endl;
	}
}
//==============================================================================