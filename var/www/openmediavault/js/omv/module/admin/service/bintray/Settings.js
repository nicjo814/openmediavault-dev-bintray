/**
 * Copyright (c) 2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

Ext.define("OMV.module.admin.service.bintray.Settings", {
    extend: "OMV.workspace.form.Panel",

    rpcService: "Bintray",
    rpcGetMethod: "getSettings",
    rpcSetMethod: "setSettings",

    initComponent: function() {
        this.on("load", function () {
            var parent = this.up("tabpanel");
            if (!parent) {
                return;
            }
            var overviewPanel = parent.down("panel[title=" + _("Overview") + "]");
            var settingsPanel = parent.down("panel[title=" + _("Settings") + "]");
            var checked = settingsPanel.findField("enabled").checked

            if (overviewPanel) {
                if (checked) {
                    overviewPanel.tab.show();
                    overviewPanel.enable();
                    parent.setActiveTab(overviewPanel);
                } else {
                    overviewPanel.disable();
                    overviewPanel.tab.hide();
                }
            }
        }, this);
        this.callParent(arguments);
    },

    getFormItems: function() {
        var me = this;
        return [{
            xtype: "fieldset",
            title: _("Bintray settings"),
            fieldDefaults: {
                labelSeparator: ""
            },
            items: [{
                xtype: "checkbox",
                name: "enabled",
                boxLabel: _("Enable the plugin")
            },{
                xtype: "textfield",
                name: "username",
                allowBlank: false,
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("Bintray username")
                }],
                listeners: {
                    change: function(field, newValue, oldValue, eOpts) {
                        me.getForm().findField("enabled").setDisabled(!(me.getForm().isValid()));
                    }
                }
            },{
                xtype: "textfield",
                inputType: "password",
                name: "apikey",
                allowBlank: false,
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("Bintray API key")
                }],
                listeners: {
                    change: function(field, newValue, oldValue, eOpts) {
                        me.getForm().findField("enabled").setDisabled(!(me.getForm().isValid()));
                    }
                }
            },{
                xtype: "textfield",
                inputType: "password",
                name: "gpgpass",
                allowBlank: false,
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("Private GPG-key passphrase")
                }],
                listeners: {
                    change: function(field, newValue, oldValue, eOpts) {
                        me.getForm().findField("enabled").setDisabled(!(me.getForm().isValid()));
                    }
                }
            }]
        }];
    },

    beforeRender: function() {
        var me = this;
        me.getForm().findField("enabled").setDisabled(!(me.getForm().isValid()));
        me.callParent(arguments);

    }
});

OMV.WorkspaceManager.registerPanel({
    id: "settings",
    path: "/service/bintray",
    text: _("Settings"),
    position: 20,
    className: "OMV.module.admin.service.bintray.Settings"
});
