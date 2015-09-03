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
// require("js/omv/workspace/panel/Panel.js")
// require("js/omv/module/admin/service/bintray/Packages.js")
// require("js/omv/module/admin/service/bintray/Publish.js")

Ext.define("OMV.module.admin.service.bintray.Overview", {
    extend: "OMV.workspace.grid.Panel",

    rpcService: "Bintray",
    rpcGetMethod: "getPackages",
    requires: [
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
    ],

    stateful: true,
    stateId: "9e468ecc-904d-427a-bda0-02e916bb796c",

    defaults: {
        flex: 1
    },
    anchor: "100%",

    columns: [{
        text: _("Version"),
        dataIndex: 'omvversion',
        sortable: true,
        stateId: 'omvversion'
    },{
        text: _("Repository"),
        dataIndex: 'repository',
        sortable: true,
        stateId: 'repository'
    },{
        text: _("Package"),
        dataIndex: 'bpackage',
        sortable: true,
        stateId: 'bpackage'
    },{
        text: _("Deb path"),
        dataIndex: 'srcpath',
        sortable: true,
        stateId: 'srcpath'
    },{
        text: _("Distribution"),
        dataIndex: 'dist',
        sortable: true,
        stateId: 'dist'
    },{
        text: _("Architecture"),
        dataIndex: 'arch',
        sortable: true,
        stateId: 'arch'
    }],

    initComponent: function() {
        var me = this;
        
        this.on("beforerender", function () {
            var parent = this.up("tabpanel");

            if (!parent) {
                return;
            }

            var overviewPanel = parent.down("panel[title=" + _("Overview") + "]");
            var settingsPanel = parent.down("panel[title=" + _("Settings") + "]");
            var checked = settingsPanel.findField("enabled").checked;

            if (overviewPanel) {
                if (checked) {
                    overviewPanel.enable();
                    overviewPanel.tab.show();
                    parent.setActiveTab(overviewPanel);
                } else {
                    overviewPanel.disable();
                    overviewPanel.tab.hide();
                    parent.setActiveTab(settingsPanel);
                }
            }
        }, this);

        Ext.apply(me, {
            store: Ext.create("OMV.data.Store", {
                autoLoad: true,
                model: OMV.data.Model.createImplicit({
                    fields: [
                        { name: "uuid", type: "string" },
                        { name: "omvversion", type: "string" },
                        { name: "repository", type: "string" },
                        { name: "bpackage", type: "string" },
                        { name: "srcpath", type: "string" },
                        { name: "dist", type: "string" },
                        { name: "arch", type: "string" }
                    ]
                }),
                proxy: {
                    type: "rpc",
                    rpcData: {
                        service: "Bintray",
                        method: "getPackages",
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    getTopToolbarItems: function(c) {
        var me = this;

        return [{
            id: me.getId() + "-add",
            xtype: "button",
            text: _("Add package"),
            icon: "images/add.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: false,
            handler: Ext.Function.bind(me.onAddButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-edit",
            xtype: "button",
            text: _("Edit package"),
            icon: "images/edit.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onEditButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-publish",
            xtype: "button",
            text: _("Publish file"),
            icon: "images/upload.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onPublishButton, me, [ me ]),
            scope: me
        }];
    },

    onSelectionChange: function(model, records) {
        var me = this;
        if(me.hideTopToolbar)
            return;
        var tbarBtnName = [ "add", "edit", "publish" ];
        var tbarBtnDisabled = {
            "add": false,
            "edit": false,
            "publish": false
        };
        // Enable/disable buttons depending on the number of selected rows.
        if(records.length <= 0) {
            tbarBtnDisabled["edit"] = true;
            tbarBtnDisabled["publish"] = true;
        } else if(records.length == 1) {
        } else {
            tbarBtnDisabled["edit"] = true;
            tbarBtnDisabled["publish"] = true;
        }

        // Update the button controls.
        Ext.Array.each(tbarBtnName, function(name) {
            var tbarBtnCtrl = me.queryById(me.getId() + "-" + name);
            if(!Ext.isEmpty(tbarBtnCtrl)) {
                if(true == tbarBtnDisabled[name]) {
                    tbarBtnCtrl.disable();
                } else {
                    tbarBtnCtrl.enable();
                }
            }
        });
    },

    onAddButton : function() {
        var me = this;
        Ext.create("OMV.module.admin.service.bintray.Packages", {
            title: _("Add package"),
            uuid: ""
        }).show();
    },
    
    onEditButton : function() {
        var me = this;
        var sm = me.getSelectionModel();
        var records = sm.getSelection();
        var record = records[0];
        Ext.create("OMV.module.admin.service.bintray.Packages", {
            title: _("Edit package"),
            uuid: record.get("uuid")
        }).show();
    },

    onPublishButton: function() {
        var me = this;
        var sm = me.getSelectionModel();
        var records = sm.getSelection();
        var record = records[0];
        Ext.create("OMV.module.admin.service.bintray.Publish", {
            uuid: record.get("uuid")
        }).show();
    }

});

OMV.WorkspaceManager.registerPanel({
    id: "overview",
    path: "/service/bintray",
    text: _("Overview"),
    position: 10,
    className: "OMV.module.admin.service.bintray.Overview"
});

