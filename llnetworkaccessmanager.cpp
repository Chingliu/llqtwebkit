/* Copyright (c) 2006-2010, Linden Research, Inc.
 *
 * LLQtWebKit Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in GPL-license.txt in this distribution, or online at
 * http://secondlifegrid.net/technology-programs/license-virtual-world/viewerlicensing/gplv2
 *
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/technology-programs/license-virtual-world/viewerlicensing/flossexception
 *
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 *
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "llnetworkaccessmanager.h"

#include <qauthenticator.h>
#include <qnetworkreply.h>
#include <qtextdocument.h>
#include <qgraphicsview.h>
#include <qgraphicsscene.h>
#include <qgraphicsproxywidget.h>
#include <qdebug.h>

#include "llembeddedbrowserwindow.h"
#include "llembeddedbrowser_p.h"

#include "ui_passworddialog.h"


LLNetworkAccessManager::LLNetworkAccessManager(LLEmbeddedBrowserPrivate* browser,QObject* parent)
    : QNetworkAccessManager(parent)
    , mBrowser(browser)
{
    connect(this, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(finishLoading(QNetworkReply*)));
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            this, SLOT(authenticationRequiredSlot(QNetworkReply*, QAuthenticator*)));
    connect(this, SIGNAL(sslErrors( QNetworkReply *, const QList<QSslError> &)),
            this, SLOT(sslErrorsSlot( QNetworkReply *, const QList<QSslError> &  )));
}

QNetworkReply *LLNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request,
                                         QIODevice *outgoingData)
{

	// Create a local copy of the request we can modify.
	QNetworkRequest mutable_request(request);

	// Set an Accept-Language header in the request, based on what the host has set through setHostLanguage.
	mutable_request.setRawHeader(QByteArray("Accept-Language"), QByteArray(mBrowser->mHostLanguage.c_str()));

	// this is undefine'd in 4.7.1 and leads to caching issues - setting it here explicitly
	mutable_request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);

	// and pass this through to the parent implementation
	return QNetworkAccessManager::createRequest(op, mutable_request, outgoingData);
}

void LLNetworkAccessManager::sslErrorsSlot(QNetworkReply* reply, const QList<QSslError>& errors)
{
	Q_UNUSED( errors );

	if ( mBrowser && mBrowser->mIgnoreSSLCertErrors )
	{
		reply->ignoreSslErrors();
	};
}

void LLNetworkAccessManager::finishLoading(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::ContentNotFoundError)
    {
        QString url = QString(reply->url().toEncoded());
        if (mBrowser)
        {
            std::string current_url = url.toStdString();
            foreach (LLEmbeddedBrowserWindow *window, mBrowser->windows)
            {
                if (window->getCurrentUri() == current_url)
                    window->load404RedirectUrl();
            }
        }
    }

	// tests if navigation request resulted in a cache hit - useful for testing so leaving here for the moment.
	//QVariant from_cache = reply->attribute( QNetworkRequest::SourceIsFromCacheAttribute );
    //QString url = QString(reply->url().toEncoded());
    //qDebug() << url << " --- from cache?" << fromCache.toBool() << "\n";
}

void LLNetworkAccessManager:: authenticationRequiredSlot(QNetworkReply *reply, QAuthenticator *authenticator)
{	
	std::string username;
	std::string password;
	std::string url = reply->url().toString().toStdString();
	std::string realm = authenticator->realm().toStdString();
	
	if(mBrowser->authRequest(url, realm, username, password))
	{
		// Got credentials to try, attempt auth with them.
		authenticator->setUser(QString::fromStdString(username));
		authenticator->setPassword(QString::fromStdString(password));
	}
	else
	{
		// The user cancelled, don't attempt auth.
	}
}

